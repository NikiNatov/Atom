#pragma once

#include "Atom/Renderer/API/Device.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"
#include "DX12DescriptorAllocator.h"
#include "Atom/Renderer/API/CommandQueue.h"

namespace Atom
{
    class DX12Device : public Device
    {
    public:
        DX12Device(GPUPreference gpuPreference);
        ~DX12Device();

        virtual void Release() override;
        virtual bool IsRayTracingSupported() const override;
        virtual u64 GetSystemMemory() const override;
        virtual u64 GetVideoMemory() const override;
        virtual u64 GetSharedMemory() const override;
        virtual String GetDescription() const override;
        virtual CommandQueue& GetCommandQueue(CommandQueueType type) override;
        virtual void ProcessDeferredReleases() override;
        virtual void WaitIdle() const override;

        DX12DescriptorHandle AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type);
        void ReleaseResource(IUnknown* resource);
        inline wrl::ComPtr<IDXGIFactory7> GetDXGIFactory() const { return m_DXGIFactory; }
        inline wrl::ComPtr<IDXGIAdapter4> GetDXGIAdapter() const { return m_DXGIAdapter; }
        inline wrl::ComPtr<ID3D12Device6> GetD3DDevice() const { return m_D3DDevice; }
    private:
        DXGI_ADAPTER_DESC3                m_Description;
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 m_FeatureOptions;
        wrl::ComPtr<IDXGIFactory7>        m_DXGIFactory;
        wrl::ComPtr<IDXGIAdapter4>        m_DXGIAdapter;
        wrl::ComPtr<ID3D12Device6>        m_D3DDevice;
        Vector<Vector<IUnknown*>>         m_DeferredReleaseResources;
                                          
        Scope<CommandQueue>               m_GraphicsQueue;
        Scope<CommandQueue>               m_ComputeQueue;
        Scope<CommandQueue>               m_CopyQueue;
        Scope<DX12DescriptorAllocator>    m_RtvAllocator;
        Scope<DX12DescriptorAllocator>    m_DsvAllocator;
        Scope<DX12DescriptorAllocator>    m_CbvSrvUavAllocator;
        Scope<DX12DescriptorAllocator>    m_SamplerAllocator;
                                          
        std::mutex                        m_DeferredReleaseMutex;
    };

}

#endif // ATOM_PLATFORM_WINDOWS