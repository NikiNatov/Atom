#include "atompch.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"

#include "Device.h"
#include "CommandQueue.h"
#include "ResourceStateTracker.h"
#include "DescriptorHeap.h"
#include "Renderer.h"

namespace Atom
{
    Device* Device::ms_Instance = nullptr;

    // -----------------------------------------------------------------------------------------------------------------------------
    Device::Device(GPUPreference gpuPreference, const char* debugName)
    {
        ATOM_ENGINE_ASSERT(ms_Instance == nullptr, "Device already exists!");
        ms_Instance = this;

        ComPtr<IDXGIAdapter> dxgiAdapter = nullptr;

        u32 factoryCreateFlags = 0;

#if defined(ATOM_DEBUG)
        factoryCreateFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif // ATOM_DEBUG

        // Create factory
        DXCall(CreateDXGIFactory2(factoryCreateFlags, IID_PPV_ARGS(&m_DXGIFactory)));

        u64 currentMaxVideoMemory = 0;

        // Query all adapters and pick the one with the most video memory
        for (u32 i = 0; m_DXGIFactory->EnumAdapterByGpuPreference(i, Utils::AtomGPUPreferenceToDXGI(gpuPreference), IID_PPV_ARGS(&dxgiAdapter)) != DXGI_ERROR_NOT_FOUND; i++)
        {
            DXGI_ADAPTER_DESC desc;
            dxgiAdapter->GetDesc(&desc);

            if (SUCCEEDED(D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) && desc.DedicatedVideoMemory > currentMaxVideoMemory)
            {
                currentMaxVideoMemory = desc.DedicatedVideoMemory;
                DXCall(dxgiAdapter.As(&m_DXGIAdapter));
            }
        }

        // Get the adapter description
        m_DXGIAdapter->GetDesc3(&m_Description);

#if defined(ATOM_DEBUG)

        // Enable debug layer
        ComPtr<ID3D12Debug> debugInterface;
        DXCall(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
        debugInterface->EnableDebugLayer();

#endif // ATOM_DEBUG

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_12_1
        };

        D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevelInfo = {};
        featureLevelInfo.NumFeatureLevels = _countof(featureLevels);
        featureLevelInfo.pFeatureLevelsRequested = featureLevels;

        // Create a device with the minimum supported feature level
        ComPtr<ID3D12Device> device = nullptr;
        DXCall(D3D12CreateDevice(m_DXGIAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));

        // Get the max supported feature level
        DXCall(device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featureLevelInfo, sizeof(featureLevelInfo)));

        // Create the main device with the max supported feature level
        ATOM_ENGINE_ASSERT(featureLevelInfo.MaxSupportedFeatureLevel >= D3D_FEATURE_LEVEL_11_0);
        DXCall(D3D12CreateDevice(m_DXGIAdapter.Get(), featureLevelInfo.MaxSupportedFeatureLevel, IID_PPV_ARGS(&m_D3DDevice)));

        // Get feature options
        DXCall(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &m_FeatureOptions, sizeof(m_FeatureOptions)));

#if defined(ATOM_DEBUG)

        String name = debugName;
        DXCall(m_D3DDevice->SetName(STRING_TO_WSTRING(name).c_str()));

        // Set up the message severity levels
        ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
        DXCall(m_D3DDevice.As(&infoQueue));

        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

        // Workaround for Windows 11 DX12 Debug Layer
        D3D12_MESSAGE_ID hide[] =
        {
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
        };

        D3D12_INFO_QUEUE_FILTER filter = {};
        filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
        filter.DenyList.pIDList = hide;
        infoQueue->AddStorageFilterEntries(&filter);

#endif // ATOM_DEBUG

        // Create command queues
        m_GraphicsQueue = CreateScope<CommandQueue>(CommandQueueType::Graphics, "GraphicsQueue");
        m_ComputeQueue = CreateScope<CommandQueue>(CommandQueueType::Compute, "ComputeQueue");
        m_CopyQueue = CreateScope<CommandQueue>(CommandQueueType::Copy, "CopyQueue");

        // Create descriptor heaps
        m_SRVHeap = CreateScope<CPUDescriptorHeap>(DescriptorHeapType::ShaderResource, Renderer::GetConfig().MaxDescriptorsPerHeap, "ShaderResourceCPUHeap");
        m_RTVHeap = CreateScope<CPUDescriptorHeap>(DescriptorHeapType::RenderTarget, Renderer::GetConfig().MaxDescriptorsPerHeap, "RenderTargetsCPUHeap");
        m_DSVHeap = CreateScope<CPUDescriptorHeap>(DescriptorHeapType::DepthStencil, Renderer::GetConfig().MaxDescriptorsPerHeap, "DepthStencilCPUHeap");
        m_SamplerHeap = CreateScope<CPUDescriptorHeap>(DescriptorHeapType::Sampler, 1024, "SamplerCPUHeap");

        m_DeferredReleaseResources.resize(Renderer::GetFramesInFlight());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Device::~Device()
    {
        // Wait for all the work on the GPU to complete
        WaitIdle();

        // Release everything
        for (u32 i = 0; i < Renderer::GetFramesInFlight(); i++)
        {
            ProcessDeferredReleases(i);
        }

#if defined(ATOM_DEBUG)

        // Reset the message severity levels
        ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
        DXCall(m_D3DDevice.As(&infoQueue));

        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);

        infoQueue.Reset();

#endif // ATOM_DEBUG

        // Release everything manually so that no references are alive when we call ReportLiveObjects()
        m_GraphicsQueue.reset();
        m_ComputeQueue.reset();
        m_CopyQueue.reset();

        m_SRVHeap.reset();
        m_RTVHeap.reset();
        m_DSVHeap.reset();
        m_SamplerHeap.reset();

        m_D3DDevice.Reset();
        m_DXGIAdapter.Reset();
        m_DXGIFactory.Reset();

#if defined (ATOM_DEBUG)

        ComPtr<IDXGIDebug1> dxgiDebug;
        DXCall(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)));
        DXCall(dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL)));

#endif // ATOM_DEBUG
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Device::WaitIdle() const
    {
        m_GraphicsQueue->Flush();
        m_ComputeQueue->Flush();
        m_CopyQueue->Flush();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool Device::IsRayTracingSupported() const
    {
        return m_FeatureOptions.RaytracingTier > D3D12_RAYTRACING_TIER_1_0;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Device::ReleaseResource(ID3D12Resource* resource, bool deferredRelease)
    {
        if (!resource)
            return;

        if (!deferredRelease)
        {
            ResourceStateTracker::RemoveGlobalResourceState(resource);
            resource->Release();
            return;
        }

        std::lock_guard<std::mutex> lock(m_DeferredReleaseMutex);
        m_DeferredReleaseResources[Renderer::GetCurrentFrameIndex()].push_back(resource);
    }
    
    // -----------------------------------------------------------------------------------------------------------------------------
    void Device::ProcessDeferredReleases(u32 frameIndex)
    {
        std::lock_guard<std::mutex> lock(m_DeferredReleaseMutex);

        // Release descriptors
        m_SRVHeap->ProcessDeferredReleases(frameIndex);
        m_RTVHeap->ProcessDeferredReleases(frameIndex);
        m_DSVHeap->ProcessDeferredReleases(frameIndex);
        m_SamplerHeap->ProcessDeferredReleases(frameIndex);

        // Release resources
        for (auto resource : m_DeferredReleaseResources[frameIndex])
        {
            ResourceStateTracker::RemoveGlobalResourceState(resource);
            resource->Release();
        }

        m_DeferredReleaseResources[frameIndex].clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    CommandQueue* Device::GetCommandQueue(CommandQueueType type) const
    {
        switch (type)
        {
            case CommandQueueType::Graphics: return m_GraphicsQueue.get();
            case CommandQueueType::Compute:  return m_ComputeQueue.get();
            case CommandQueueType::Copy:     return m_CopyQueue.get();
        }

        ATOM_ENGINE_ASSERT(false, "Unknown queue type!");
        return m_GraphicsQueue.get();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    CPUDescriptorHeap* Device::GetCPUDescriptorHeap(DescriptorHeapType type) const
    {
        switch (type)
        {
            case DescriptorHeapType::ShaderResource: return m_SRVHeap.get();
            case DescriptorHeapType::RenderTarget:   return m_RTVHeap.get();
            case DescriptorHeapType::DepthStencil:   return m_DSVHeap.get();
            case DescriptorHeapType::Sampler:        return m_SamplerHeap.get();
        }

        ATOM_ENGINE_ASSERT(false, "Unknown heap type!");
        return m_SRVHeap.get();
    }
}
