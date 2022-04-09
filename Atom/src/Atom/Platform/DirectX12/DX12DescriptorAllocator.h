#pragma once

#include "Atom/Core/Core.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"

namespace Atom
{
    class DX12DescriptorHeap;

    class DX12DescriptorHandle
    {
        friend class DX12DescriptorHeap;
    public:
        DX12DescriptorHandle();
        DX12DescriptorHandle(DX12DescriptorHandle&& other);
        DX12DescriptorHandle& operator=(DX12DescriptorHandle&& other);
        ~DX12DescriptorHandle();

        void Release();
        void DeferredRelease();

        inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return m_CPUHandle; }
        inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return m_GPUHandle; }
        inline bool IsValid() const { return m_CPUHandle.ptr != 0; }
        inline bool IsShaderVisible() const { return m_GPUHandle.ptr != 0; }
    private:
        DX12DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle, DX12DescriptorHeap* heap);
    private:
        D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle{ 0 };
        D3D12_GPU_DESCRIPTOR_HANDLE m_GPUHandle{ 0 };
        DX12DescriptorHeap*         m_Heap;
    };

    class DX12DescriptorHeap
    {
    public:
        DX12DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, u32 capacity, bool shaderVisible);
        ~DX12DescriptorHeap();

        DX12DescriptorHandle Allocate();
        void ReleaseDescriptor(DX12DescriptorHandle& descriptor);
        void DeferredReleaseDescriptor(DX12DescriptorHandle& descriptor);
        void ProcessDeferredReleases(u32 frameIndex);

        inline bool IsShaderVisible() const { return m_GPUStartHandle.ptr != 0; }
        inline D3D12_DESCRIPTOR_HEAP_TYPE GetType() const { return m_Type; }
        inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPUStartHandle() const { return m_CPUStartHandle; }
        inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPUStartHandle() const { return m_GPUStartHandle; }
        inline u32 GetCapacity() const { return m_Capacity; }
        inline u32 GetSize() const { return m_Size; }
        inline u32 GetDescriptorSize() const { return m_DescriptorSize; }
    private:
        ComPtr<ID3D12DescriptorHeap> m_D3DHeap;
        D3D12_DESCRIPTOR_HEAP_TYPE   m_Type;
        D3D12_CPU_DESCRIPTOR_HANDLE  m_CPUStartHandle{ 0 };
        D3D12_GPU_DESCRIPTOR_HANDLE  m_GPUStartHandle{ 0 };
        u32                          m_Capacity;
        u32                          m_Size;
        u32                          m_DescriptorSize;
        Queue<u32>                   m_FreeSlots;
        Vector<Vector<u32>>          m_DeferredReleaseDescriptors;
        std::mutex                   m_Mutex;
    };

    class DX12DescriptorAllocator
    {
    public:
        DX12DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE descriptorType, u32 heapMaxCapacity, bool shaderVisible);
        ~DX12DescriptorAllocator();

        DX12DescriptorHandle Allocate();
        void ProcessDeferredReleases(u32 frameIndex);

        inline D3D12_DESCRIPTOR_HEAP_TYPE GetType() const { return m_HeapType; }
        inline u32 GetHeapCapacity() const { return m_HeapCapacity; }
        inline bool IsShaderVisible() const { return m_ShaderVisible; }
    private:
        D3D12_DESCRIPTOR_HEAP_TYPE      m_HeapType;
        u32                             m_HeapCapacity;
        bool                            m_ShaderVisible;
        Vector<Ref<DX12DescriptorHeap>> m_HeapPool;
        Set<u32>                        m_AvailableHeaps;
        std::mutex                      m_Mutex;
    };
}

#endif // ATOM_PLATFORM_WINDOWS