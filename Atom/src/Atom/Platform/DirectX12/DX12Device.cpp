#include "atompch.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DX12Device.h"
#include "DX12ResourceStateTracker.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    DX12Device::DX12Device(GPUPreference gpuPreference)
    {
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
        DXCall(m_D3DDevice->SetName(L"Main Device"));

        // Get feature options
        DXCall(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &m_FeatureOptions, sizeof(m_FeatureOptions)));

#if defined(ATOM_DEBUG)

        // Set up the message severity levels
        ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
        DXCall(m_D3DDevice.As(&infoQueue));

        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        //infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

#endif // ATOM_DEBUG
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12Device::~DX12Device()
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

        m_RtvAllocator.reset();
        m_DsvAllocator.reset();
        m_CbvSrvUavAllocator.reset();
        m_SamplerAllocator.reset();

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
    void DX12Device::Initialize()
    {
        // Create command queues
        m_GraphicsQueue = CommandQueue::Create(CommandQueueType::Graphics);
        m_ComputeQueue = CommandQueue::Create(CommandQueueType::Compute);
        m_CopyQueue = CommandQueue::Create(CommandQueueType::Copy);

        // Create descriptor allocators
        m_RtvAllocator = CreateScope<DX12DescriptorAllocator>(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 512, false);
        m_DsvAllocator = CreateScope<DX12DescriptorAllocator>(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 512, false);
        m_CbvSrvUavAllocator = CreateScope<DX12DescriptorAllocator>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096, true);
        m_SamplerAllocator = CreateScope<DX12DescriptorAllocator>(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 512, true);

        // Create deferred release arrays for resources
        m_DeferredReleaseResources.resize(Renderer::GetFramesInFlight());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12Device::ProcessDeferredReleases(u32 frameIndex)
    {
        std::lock_guard<std::mutex> lock(m_DeferredReleaseMutex);

        // Release descriptors
        m_RtvAllocator->ProcessDeferredReleases(frameIndex);
        m_DsvAllocator->ProcessDeferredReleases(frameIndex);
        m_CbvSrvUavAllocator->ProcessDeferredReleases(frameIndex);
        m_SamplerAllocator->ProcessDeferredReleases(frameIndex);

        // Release resources
        for (auto resource : m_DeferredReleaseResources[frameIndex])
        {
            DX12ResourceStateTracker::RemoveGlobalResourceState((ID3D12Resource*)resource);
            resource->Release();
        }

        m_DeferredReleaseResources[frameIndex].clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12Device::WaitIdle() const
    {
        m_GraphicsQueue->Flush();
        m_ComputeQueue->Flush();
        m_CopyQueue->Flush();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool DX12Device::IsRayTracingSupported() const
    {
        return m_FeatureOptions.RaytracingTier > D3D12_RAYTRACING_TIER_1_0;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u64 DX12Device::GetSystemMemory() const
    {
        return m_Description.DedicatedSystemMemory;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u64 DX12Device::GetVideoMemory() const
    {
        return m_Description.DedicatedVideoMemory;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u64 DX12Device::GetSharedMemory() const
    {
        return m_Description.SharedSystemMemory;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    String DX12Device::GetDescription() const
    {
        std::wstring desc = m_Description.Description;
        return std::string(desc.begin(), desc.end());;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    CommandQueue& DX12Device::GetCommandQueue(CommandQueueType type)
    {
        switch (type)
        {
            case CommandQueueType::Graphics: return *m_GraphicsQueue;
            case CommandQueueType::Compute:  return *m_ComputeQueue;
            case CommandQueueType::Copy:     return *m_CopyQueue;
        }

        ATOM_ENGINE_ASSERT(false, "Unknown queue type!");
        return *m_GraphicsQueue;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12DescriptorHandle DX12Device::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type)
    {
        switch (type)
        {
            case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: return m_CbvSrvUavAllocator->Allocate();
            case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:         return m_DsvAllocator->Allocate();
            case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:         return m_RtvAllocator->Allocate();
            case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:     return m_SamplerAllocator->Allocate();
        }

        ATOM_ENGINE_ASSERT(false, "Unknown descriptor type!");
        return DX12DescriptorHandle();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12Device::ReleaseResource(IUnknown* resource)
    {
        std::lock_guard<std::mutex> lock(m_DeferredReleaseMutex);
        m_DeferredReleaseResources[Renderer::GetCurrentFrameIndex()].push_back(resource);
    }
}

#endif // ATOM_PLATFORM_WINDOWS
