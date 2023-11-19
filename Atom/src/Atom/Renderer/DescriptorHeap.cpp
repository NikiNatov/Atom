#include "atompch.h"
#include "DescriptorHeap.h"

#include "Atom/Core/Application.h"
#include "Atom/Core/DirectX12/DirectX12Utils.h"

#include "Device.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    DescriptorHeap::DescriptorHeap(DescriptorHeapType type, u32 capacity, bool shaderVisible, const char* debugName)
        : m_Type(type), m_Capacity(capacity), m_Size(0)
    {
        ATOM_ENGINE_ASSERT(capacity && ((m_Type != DescriptorHeapType::Sampler && capacity <= D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2) ||
                                        (m_Type == DescriptorHeapType::Sampler && capacity <= D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE)), "Invalid capacity!");

        auto d3dDevice = Device::Get().GetD3DDevice();

        // RTV and DSV heaps cannot be shader visible
        if (m_Type == DescriptorHeapType::RenderTarget || m_Type == DescriptorHeapType::DepthStencil)
        {
            shaderVisible = false;
        }

        // Create the heap
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.Type = Utils::AtomDescriptorHeapTypeToD3D12(m_Type);
        heapDesc.NumDescriptors = capacity;
        heapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        heapDesc.NodeMask = 0;

        DXCall(d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_D3DHeap)));

#if defined (ATOM_DEBUG)
        String name = debugName;
        DXCall(m_D3DHeap->SetName(STRING_TO_WSTRING(name).c_str()));
#endif

        // Get the descriptor size
        m_DescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(Utils::AtomDescriptorHeapTypeToD3D12(m_Type));

        // Get the CPU and GPU heap start handles
        m_CPUStartHandle = m_D3DHeap->GetCPUDescriptorHandleForHeapStart();
        m_GPUStartHandle = shaderVisible ? m_D3DHeap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DescriptorHeap::~DescriptorHeap()
    {
    }

    // ------------------------------------------------ DescriptorAllocator --------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    DescriptorAllocator::DescriptorAllocator(DescriptorAllocationType type, DescriptorHeap& heap, u32 heapOffset, u32 capacity)
        : m_Type(type), m_HeapOffset(heapOffset), m_Capacity(capacity), m_AllocatedDescriptorCount(0), m_Heap(heap)
    {
        // Add the hole rage of descriptors as free block
        AddBlock(0, m_Capacity);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DescriptorAllocation DescriptorAllocator::Allocate(u32 descriptorCount)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        ATOM_ENGINE_ASSERT(m_AllocatedDescriptorCount + descriptorCount <= m_Capacity, "Allocator has no space");

        auto firstSuitableBlockIt = m_FreeBlocksBySize.lower_bound(descriptorCount);
        ATOM_ENGINE_ASSERT(firstSuitableBlockIt != m_FreeBlocksBySize.end());

        u32 blockSize = firstSuitableBlockIt->first;
        u32 blockOffset = firstSuitableBlockIt->second.FreeBlocksByOffsetIt->first;

        // Erase the current block and create a new one with adjusted offset and size
        m_FreeBlocksByOffset.erase(firstSuitableBlockIt->second.FreeBlocksByOffsetIt);
        m_FreeBlocksBySize.erase(firstSuitableBlockIt);

        u32 newOffset = blockOffset + descriptorCount;
        u32 newSize = blockSize - descriptorCount;

        if (newSize > 0)
            AddBlock(newOffset, newSize);

        m_AllocatedDescriptorCount += descriptorCount;

        return DescriptorAllocation(m_Type, descriptorCount, m_HeapOffset + blockOffset, &m_Heap);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DescriptorAllocator::Release(DescriptorAllocation&& allocation, bool deferredRelease)
    {
        // Only persistent allocations get queued for deferred release, transient ones get released every frame
        if (m_Type == DescriptorAllocationType::Persistent)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_DeferredReleaseAllocations[Application::Get().GetCurrentFrameIndex()].emplace_back(allocation);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DescriptorAllocator::ProcessDeferredReleases(u32 frameIndex)
    {
        // Only persistent allocations get queued for deferred release, transient ones get released every frame
        if (m_Type == DescriptorAllocationType::Persistent)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);

            for (auto& allocation : m_DeferredReleaseAllocations[frameIndex])
            {
                u32 size = allocation.GetSize();
                u32 offset = allocation.GetHeapOffset() - m_HeapOffset;
                FreeBlock(offset, size);
            }

            m_DeferredReleaseAllocations[frameIndex].clear();
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DescriptorAllocator::Reset()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        m_AllocatedDescriptorCount = 0;
        m_FreeBlocksByOffset.clear();
        m_FreeBlocksBySize.clear();

        AddBlock(0, m_Capacity);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DescriptorAllocator::AddBlock(u32 offset, u32 size)
    {
        auto freeBlocksByOffsetIt = m_FreeBlocksByOffset.emplace(offset, size).first;
        auto freeBlocksBySizeIt = m_FreeBlocksBySize.emplace(size, freeBlocksByOffsetIt);
        freeBlocksByOffsetIt->second.FreeBlocksBySizeIt = freeBlocksBySizeIt;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DescriptorAllocator::FreeBlock(u32 offset, u32 size)
    {
        // Find the first free block that appears after the one we are freeing
        auto nextFreeBlockIt = m_FreeBlocksByOffset.upper_bound(offset);

        // Find the first free block that appears before the one we are freeing
        auto prevFreeBlockIt = nextFreeBlockIt;
        if (nextFreeBlockIt != m_FreeBlocksByOffset.begin())
        {
            --prevFreeBlockIt;
        }
        else
        {
            // No free blocks exist before the one we are freeing
            prevFreeBlockIt = m_FreeBlocksByOffset.end();
        }

        u32 newFreeBlockSize = size;
        u32 newFreeBlockOffset = offset;

        if (prevFreeBlockIt != m_FreeBlocksByOffset.end())
        {
            u32 prevFreeBlockOffset = prevFreeBlockIt->first;
            u32 prevFreeBlockSize = prevFreeBlockIt->second.BlockSize;

            // The offset of the block we are freeing is right at the end of the previous free block so we can merge them
            if (newFreeBlockOffset == prevFreeBlockOffset + prevFreeBlockSize)
            {
                newFreeBlockOffset = prevFreeBlockOffset;
                newFreeBlockSize += prevFreeBlockSize;

                m_FreeBlocksBySize.erase(prevFreeBlockIt->second.FreeBlocksBySizeIt);
                m_FreeBlocksByOffset.erase(prevFreeBlockIt);
            }
        }

        if (nextFreeBlockIt != m_FreeBlocksByOffset.end())
        {
            u32 nextFreeBlockOffset = nextFreeBlockIt->first;
            u32 nextFreeBlockSize = nextFreeBlockIt->second.BlockSize;

            // The end of the block we are freeing is right at the beginning of the next free block so we can merge them
            if (nextFreeBlockOffset == newFreeBlockOffset + newFreeBlockSize)
            {
                newFreeBlockSize += nextFreeBlockSize;

                m_FreeBlocksBySize.erase(nextFreeBlockIt->second.FreeBlocksBySizeIt);
                m_FreeBlocksByOffset.erase(nextFreeBlockIt);
            }
        }

        AddBlock(newFreeBlockOffset, newFreeBlockSize);

        m_AllocatedDescriptorCount -= size;
    }

    // -------------------------------------------------- CPUDescriptorHeap --------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    CPUDescriptorHeap::CPUDescriptorHeap(DescriptorHeapType type, u32 capacity, const char* debugName)
        : DescriptorHeap(type, capacity, false, debugName)
    {
        // Initialize the free slot indices
        for (u32 i = 0; i < m_Capacity; i++)
        {
            m_FreeSlots.push(i);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    CPUDescriptorHeap::~CPUDescriptorHeap()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHeap::AllocateDescriptor()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        ATOM_ENGINE_ASSERT(m_Size < m_Capacity, "Heap is full!");

        u32 descriptorIndex = m_FreeSlots.front();
        m_FreeSlots.pop();
        m_Size++;

        return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CPUStartHandle, descriptorIndex, m_DescriptorSize);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CPUDescriptorHeap::ReleaseDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, bool deferredRelease)
    {
        if (descriptor.ptr != 0)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);

            ATOM_ENGINE_ASSERT((descriptor.ptr - m_CPUStartHandle.ptr) % m_DescriptorSize == 0, "Descriptor has different type than the heap!");
            ATOM_ENGINE_ASSERT(descriptor.ptr >= m_CPUStartHandle.ptr &&
                descriptor.ptr < m_CPUStartHandle.ptr + m_DescriptorSize * m_Capacity, "Descriptor does not belong to this heap!");

            // Return the index back to the free slots array
            u32 descriptorIndex = (descriptor.ptr - m_CPUStartHandle.ptr) / m_DescriptorSize;

            if (deferredRelease)
            {
                m_DeferredReleaseDescriptors[Application::Get().GetCurrentFrameIndex()].push_back(descriptorIndex);
            }
            else
            {
                m_Size--;
                m_FreeSlots.push(descriptorIndex);
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CPUDescriptorHeap::ProcessDeferredReleases(u32 frameIndex)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        if (!m_DeferredReleaseDescriptors[frameIndex].empty())
        {
            for (auto index : m_DeferredReleaseDescriptors[frameIndex])
            {
                // Return the index back to the free slots array
                m_Size--;
                m_FreeSlots.push(index);
            }

            m_DeferredReleaseDescriptors[frameIndex].clear();
        }
    }

    // -------------------------------------------------- GPUDescriptorHeap --------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    GPUDescriptorHeap::GPUDescriptorHeap(DescriptorHeapType type, u32 persistentBlockSize, u32 transientBlockSize, const char* debugName)
        : DescriptorHeap(type, persistentBlockSize + transientBlockSize * g_FramesInFlight, true, debugName)
    {
        m_PersistentAllocator = CreateScope<DescriptorAllocator>(DescriptorAllocationType::Persistent, *this, 0, persistentBlockSize);

        for (u32 i = 0; i < g_FramesInFlight; i++)
            m_TransientAllocators[i] = CreateScope<DescriptorAllocator>(DescriptorAllocationType::Transient, *this, persistentBlockSize + transientBlockSize * i, transientBlockSize);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DescriptorAllocation GPUDescriptorHeap::AllocatePersistent(u32 descriptorCount)
    {
        return m_PersistentAllocator->Allocate(descriptorCount);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DescriptorAllocation GPUDescriptorHeap::AllocateTransient(u32 descriptorCount)
    {
        return m_TransientAllocators[Application::Get().GetCurrentFrameIndex()]->Allocate(descriptorCount);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void GPUDescriptorHeap::Release(DescriptorAllocation&& allocation, bool deferredRelease)
    {
        if (!allocation.IsValid() || allocation.GetType() == DescriptorAllocationType::Transient)
            return;

        ATOM_ENGINE_ASSERT((allocation.GetBaseCpuDescriptor().ptr - m_CPUStartHandle.ptr) % m_DescriptorSize == 0, "Descriptor allocation has different type than the heap!");
        ATOM_ENGINE_ASSERT(allocation.GetBaseCpuDescriptor().ptr >= m_CPUStartHandle.ptr &&
            allocation.GetBaseCpuDescriptor().ptr < m_CPUStartHandle.ptr + m_DescriptorSize * m_Capacity, "Descriptor allocation does not belong to this heap!");
        ATOM_ENGINE_ASSERT(allocation.IsShaderVisible());

        if (allocation.GetHeapOffset() < m_PersistentAllocator->GetCapacity())
            m_PersistentAllocator->Release(std::move(allocation), deferredRelease);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void GPUDescriptorHeap::ProcessDeferredReleases(u32 frameIndex)
    {
        m_PersistentAllocator->ProcessDeferredReleases(frameIndex);
        m_TransientAllocators[frameIndex]->Reset();
    }
}