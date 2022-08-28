#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

namespace Atom
{
    enum class DescriptorHeapType
    {
        ShaderResource,
        RenderTarget,
        DepthStencil,
        Sampler,
        TypeCount = Sampler
    };

    class DescriptorHeap
    {
    public:
        DescriptorHeap(DescriptorHeapType type, u32 capacity, bool shaderVisible, const char* debugName = "Unnamed Descriptor Heap");
        virtual ~DescriptorHeap();

        D3D12_GPU_DESCRIPTOR_HANDLE CopyDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE descriptor);
        D3D12_GPU_DESCRIPTOR_HANDLE CopyDescriptors(D3D12_CPU_DESCRIPTOR_HANDLE* descriptors, u32 descriptorCount);
        inline ComPtr<ID3D12DescriptorHeap> GetD3DHeap() const { return m_D3DHeap; }
        inline DescriptorHeapType GetType() const { return m_Type; }
        inline u32 GetSize() const { return m_Size; }
        inline u32 GetDescriptorSize() const { return m_DescriptorSize; }
        inline u32 GetCapacity() const { return m_Capacity; }
        inline bool IsShaderVisible() const { return m_GPUStartHandle.ptr != 0; }
        inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPUStartHandle() const { return m_CPUStartHandle; }
        inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPUStartHandle() const { return m_GPUStartHandle; }
    protected:
        ComPtr<ID3D12DescriptorHeap> m_D3DHeap;
        DescriptorHeapType           m_Type;
        D3D12_CPU_DESCRIPTOR_HANDLE  m_CPUStartHandle{ 0 };
        D3D12_GPU_DESCRIPTOR_HANDLE  m_GPUStartHandle{ 0 };
        u32                          m_Capacity;
        u32                          m_Size;
        u32                          m_DescriptorSize;
    };

    class CPUDescriptorHeap : public DescriptorHeap
    {
    public:
        CPUDescriptorHeap(DescriptorHeapType type, u32 capacity, const char* debugName = "Unnamed CPU Descriptor Heap");
        ~CPUDescriptorHeap();

        D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor();
        void ReleaseDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, bool deferredRelease);
        void ProcessDeferredReleases(u32 frameIndex);
    private:
        Queue<u32>          m_FreeSlots;
        Vector<Vector<u32>> m_DeferredReleaseDescriptors;
        std::mutex          m_Mutex;
    };
}