#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

#include "CommandQueue.h"
#include "DescriptorHeap.h"

namespace Atom
{
    enum class GPUPreference
    {
        None = 0,
        MinimumPowered = 1,
        HighPerformance = 2
    };

    class Device
    {
    public:
        Device(GPUPreference gpuPreference, const char* debugName = "Unnamed Device");
        ~Device();

        void Initialize();
        void WaitIdle() const;
        void ReleaseResource(ID3D12Resource* resource, bool deferredRelease);
        void ProcessDeferredReleases(u32 frameIndex);
        CommandQueue* GetCommandQueue(CommandQueueType type) const;
        CPUDescriptorHeap* GetCPUDescriptorHeap(DescriptorHeapType type) const;
        bool IsRayTracingSupported() const;
        inline ComPtr<IDXGIFactory7> GetDXGIFactory() const { return m_DXGIFactory; }
        inline ComPtr<IDXGIAdapter4> GetDXGIAdapter() const { return m_DXGIAdapter; }
        inline ComPtr<ID3D12Device6> GetD3DDevice() const { return m_D3DDevice; }
    private:
        DXGI_ADAPTER_DESC3                m_Description;
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 m_FeatureOptions;
        ComPtr<IDXGIFactory7>             m_DXGIFactory;
        ComPtr<IDXGIAdapter4>             m_DXGIAdapter;
        ComPtr<ID3D12Device6>             m_D3DDevice;

        // Command queues
        Scope<CommandQueue>               m_GraphicsQueue;
        Scope<CommandQueue>               m_ComputeQueue;
        Scope<CommandQueue>               m_CopyQueue;

        // Descriptor heaps
        Scope<CPUDescriptorHeap>          m_SRVHeap;
        Scope<CPUDescriptorHeap>          m_RTVHeap;
        Scope<CPUDescriptorHeap>          m_DSVHeap;
        Scope<CPUDescriptorHeap>          m_SamplerHeap;

        // Resource release
        Vector<Vector<ID3D12Resource*>>   m_DeferredReleaseResources;
        std::mutex                        m_DeferredReleaseMutex;
    };
}