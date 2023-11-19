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

        inline DescriptorHeapType GetType() const { return m_Type; }
        inline u32 GetDescriptorSize() const { return m_DescriptorSize; }
        inline u32 GetCapacity() const { return m_Capacity; }
        inline bool IsShaderVisible() const { return m_GPUStartHandle.ptr != 0; }
        inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPUStartHandle() const { return m_CPUStartHandle; }
        inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPUStartHandle() const { return m_GPUStartHandle; }
        inline ComPtr<ID3D12DescriptorHeap> GetD3DHeap() const { return m_D3DHeap; }
    protected:
        DescriptorHeapType           m_Type;
        D3D12_CPU_DESCRIPTOR_HANDLE  m_CPUStartHandle{ 0 };
        D3D12_GPU_DESCRIPTOR_HANDLE  m_GPUStartHandle{ 0 };
        u32                          m_Capacity;
        u32                          m_Size;
        u32                          m_DescriptorSize;
        ComPtr<ID3D12DescriptorHeap> m_D3DHeap;
    };

    enum class DescriptorAllocationType
    {
        None = 0,
        Persistent,
        Transient
    };

    class DescriptorAllocation
    {
    public:
        
    public:
        DescriptorAllocation() = default;
        DescriptorAllocation(DescriptorAllocationType type, u32 size, u32 heapOffset, const DescriptorHeap* heap)
            : m_Type(type), m_Size(size), m_HeapOffset(heapOffset), m_Heap(heap)
        {
            m_BaseCpuDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_Heap->GetCPUStartHandle(), m_HeapOffset, m_Heap->GetDescriptorSize());
            m_BaseGpuDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_Heap->GetGPUStartHandle(), m_HeapOffset, m_Heap->GetDescriptorSize());
        }

        bool IsValid() const { return m_BaseCpuDescriptor.ptr != 0; }
        bool IsShaderVisible() const { return m_BaseGpuDescriptor.ptr != 0; }
        DescriptorAllocationType GetType() const { return m_Type; }
        u32 GetSize() const { return m_Size; }
        u32 GetHeapOffset() const { return m_HeapOffset; }
        D3D12_CPU_DESCRIPTOR_HANDLE GetBaseCpuDescriptor() const { return m_BaseCpuDescriptor; }
        D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptor(u32 index) const { return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_BaseCpuDescriptor, index, m_Heap->GetDescriptorSize()); }
        D3D12_GPU_DESCRIPTOR_HANDLE GetBaseGpuDescriptor() const { return m_BaseGpuDescriptor; }
        D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptor(u32 index) const { return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_BaseGpuDescriptor, index, m_Heap->GetDescriptorSize()); }

        inline operator bool() const { return IsValid(); }
    private:
        DescriptorAllocationType    m_Type = DescriptorAllocationType::None;
        u32                         m_Size = 0;
        u32                         m_HeapOffset = 0;
        D3D12_CPU_DESCRIPTOR_HANDLE m_BaseCpuDescriptor{ 0 };
        D3D12_GPU_DESCRIPTOR_HANDLE m_BaseGpuDescriptor{ 0 };
        const DescriptorHeap*       m_Heap = nullptr;
    };

    class DescriptorAllocator
    {
    public:
        DescriptorAllocator(DescriptorAllocationType type, DescriptorHeap& heap, u32 heapOffset, u32 capacity);

        DescriptorAllocation Allocate(u32 descriptorCount);
        void Release(DescriptorAllocation&& allocation, bool deferredRelease);
        void ProcessDeferredReleases(u32 frameIndex);
        void Reset();

        inline DescriptorAllocationType GetType() const { return m_Type; }
        inline u32 GetHeapOffset() const { return m_HeapOffset; }
        inline u32 GetCapacity() const { return m_Capacity; }
        inline u32 GetFreeDescriptorCount() const { return m_Capacity - m_AllocatedDescriptorCount; }
        inline const DescriptorHeap& GetParentHeap() const { return m_Heap; }
    private:
        void AddBlock(u32 offset, u32 size);
        void FreeBlock(u32 offset, u32 size);
    private:
        struct FreeBlocksByOffsetEntry;
        struct FreeBlocksBySizeEntry;

        using FreeBlocksByOffsetIter = Map<u32, FreeBlocksByOffsetEntry>::iterator;
        using FreeBlocksBySizeIter = MultiMap<u32, FreeBlocksBySizeEntry>::iterator;

        struct FreeBlocksByOffsetEntry
        {
            u32 BlockSize;
            FreeBlocksBySizeIter FreeBlocksBySizeIt;

            FreeBlocksByOffsetEntry(u32 size)
                : BlockSize(size) {}
        };

        struct FreeBlocksBySizeEntry
        {
            FreeBlocksByOffsetIter FreeBlocksByOffsetIt;

            FreeBlocksBySizeEntry(FreeBlocksByOffsetIter iter)
                : FreeBlocksByOffsetIt(iter) {}
        };

        DescriptorAllocationType             m_Type;
        u32                                  m_HeapOffset;
        u32                                  m_Capacity;
        u32                                  m_AllocatedDescriptorCount;
        DescriptorHeap&                      m_Heap;
        Map<u32, FreeBlocksByOffsetEntry>    m_FreeBlocksByOffset;
        MultiMap<u32, FreeBlocksBySizeEntry> m_FreeBlocksBySize;
        Vector<DescriptorAllocation>         m_DeferredReleaseAllocations[g_FramesInFlight];
        std::mutex                           m_Mutex;
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
        Queue<u32>  m_FreeSlots;
        Vector<u32> m_DeferredReleaseDescriptors[g_FramesInFlight];
        std::mutex  m_Mutex;
    };

    class GPUDescriptorHeap : public DescriptorHeap
    {
    public:
        GPUDescriptorHeap(DescriptorHeapType type, u32 persistentBlockSize, u32 transientBlockSize, const char* debugName = "Unnamed GPU Descriptor Heap");

        DescriptorAllocation AllocatePersistent(u32 descriptorCount);
        DescriptorAllocation AllocateTransient(u32 descriptorCount);
        void Release(DescriptorAllocation&& allocation, bool deferredRelease);
        void ProcessDeferredReleases(u32 frameIndex);
    private:
        Scope<DescriptorAllocator> m_PersistentAllocator = nullptr;
        Scope<DescriptorAllocator> m_TransientAllocators[g_FramesInFlight];
    };
}