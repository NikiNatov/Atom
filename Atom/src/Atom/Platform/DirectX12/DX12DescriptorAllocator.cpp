#include "atompch.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DX12DescriptorAllocator.h"
#include "DX12Device.h"

#include "Atom/Renderer/API/Renderer.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    DX12DescriptorHandle::DX12DescriptorHandle()
        : m_CPUHandle{ 0 }, m_GPUHandle{ 0 }, m_Heap(nullptr)
    {
    }

    // ----------------------------------------------------- DX12DescriptorHandle --------------------------------------------------
    DX12DescriptorHandle::DX12DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle, DX12DescriptorHeap* heap)
        : m_CPUHandle(cpuHandle), m_GPUHandle(gpuHandle), m_Heap(heap)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12DescriptorHandle::DX12DescriptorHandle(DX12DescriptorHandle&& other)
    {
        m_CPUHandle = other.m_CPUHandle;
        m_GPUHandle = other.m_GPUHandle;
        m_Heap = other.m_Heap;

        other.m_CPUHandle.ptr = 0;
        other.m_GPUHandle.ptr = 0;
        other.m_Heap = nullptr;
    }

    DX12DescriptorHandle& DX12DescriptorHandle::operator=(DX12DescriptorHandle&& other)
    {
        if (this != &other)
        {
            m_CPUHandle = other.m_CPUHandle;
            m_GPUHandle = other.m_GPUHandle;
            m_Heap = other.m_Heap;

            other.m_CPUHandle.ptr = 0;
            other.m_GPUHandle.ptr = 0;
            other.m_Heap = nullptr;
        }

        return *this;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12DescriptorHandle::~DX12DescriptorHandle()
    {
        if (IsValid())
        {
            m_Heap->FreeDescriptor(*this);
        }
    }

    // ----------------------------------------------------- DX12DescriptorHeap ----------------------------------------------------
    DX12DescriptorHeap::DX12DescriptorHeap(wrl::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 capacity, bool shaderVisible)
        : m_Type(type), m_Capacity(capacity), m_Size(0)
    {
        ATOM_ENGINE_ASSERT(capacity && 
            ((type != D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER && capacity <= D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2) || 
            (type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER && capacity <= D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE)), "Invalid capacity!");

        // RTV and DSV heaps cannot be shader visible
        if (type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
        {
            shaderVisible = false;
        }

        // Create the heap
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.Type = m_Type;
        heapDesc.NumDescriptors = capacity;
        heapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        heapDesc.NodeMask = 0;

        DXCall(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_D3DHeap)));

        // Initialize the free slot indices
        for (u32 i = 0; i < m_Capacity; i++)
        {
            m_FreeSlots.push(i);
        }

        // Get the descriptor size
        m_DescriptorSize = device->GetDescriptorHandleIncrementSize(m_Type);

        // Get the CPU and GPU heap start handles
        m_CPUStartHandle = m_D3DHeap->GetCPUDescriptorHandleForHeapStart();
        m_GPUStartHandle = shaderVisible ? m_D3DHeap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };

        // Create arrays for deferred release descriptors for each frame in flight
        m_DeferredReleaseDescriptors.resize(Renderer::GetFramesInFlight());
    }
    
    // -----------------------------------------------------------------------------------------------------------------------------
    DX12DescriptorHeap::~DX12DescriptorHeap()
    {

    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12DescriptorHandle DX12DescriptorHeap::Allocate()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        ATOM_ENGINE_ASSERT(m_Size < m_Capacity, "Heap is full!");

        u32 descriptorIndex = m_FreeSlots.front();
        m_FreeSlots.pop();
        m_Size++;

        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CPUStartHandle, descriptorIndex, m_DescriptorSize);
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = IsShaderVisible() ? CD3DX12_GPU_DESCRIPTOR_HANDLE(m_GPUStartHandle, descriptorIndex, m_DescriptorSize) : D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };

        return DX12DescriptorHandle(cpuHandle, gpuHandle, this);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12DescriptorHeap::FreeDescriptor(DX12DescriptorHandle& descriptor)
    {
        if (descriptor.IsValid())
        {
            std::lock_guard<std::mutex> lock(m_Mutex);

            ATOM_ENGINE_ASSERT((descriptor.GetCPUHandle().ptr - m_CPUStartHandle.ptr) % m_DescriptorSize == 0, "Descriptor has different type than the heap!");
            ATOM_ENGINE_ASSERT(descriptor.GetCPUHandle().ptr >= m_CPUStartHandle.ptr &&
                               descriptor.GetCPUHandle().ptr < m_CPUStartHandle.ptr + m_DescriptorSize * m_Capacity, "Descriptor does not belong to this heap!");

            // Queue the descriptor for deferred release for the current frame
            u32 descriptorIndex = (descriptor.GetCPUHandle().ptr - m_CPUStartHandle.ptr) % m_DescriptorSize;
            m_DeferredReleaseDescriptors[Renderer::GetCurrentFrameIndex()].push_back(descriptorIndex);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12DescriptorHeap::ReleaseDescriptors()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        u32 frameIndex = Renderer::GetCurrentFrameIndex();

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


    // -------------------------------------------------- DX12DescriptorAllocator --------------------------------------------------
    DX12DescriptorAllocator::DX12DescriptorAllocator(wrl::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE descriptorType, u32 heapCapacity, bool shaderVisible)
        : m_HeapType(descriptorType), m_HeapCapacity(heapCapacity), m_ShaderVisible(shaderVisible), m_Device(device)
    {
        ATOM_ENGINE_ASSERT(m_HeapCapacity > 0, "Heap capacity must be greater that 0!");

        // Create a starting heap
        m_HeapPool.emplace_back(CreateRef<DX12DescriptorHeap>(m_Device, m_HeapType, m_HeapCapacity, m_ShaderVisible));
        m_AvailableHeaps.push(m_HeapPool.size() - 1);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12DescriptorAllocator::~DX12DescriptorAllocator()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12DescriptorHandle DX12DescriptorAllocator::Allocate()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        if (m_AvailableHeaps.empty())
        {
            // If there are no available heaps, create a new one
            m_HeapPool.emplace_back(CreateRef<DX12DescriptorHeap>(m_Device, m_HeapType, m_HeapCapacity, m_ShaderVisible));
            m_AvailableHeaps.push(m_HeapPool.size() - 1);
        }

        DX12DescriptorHeap& heap = *m_HeapPool[m_AvailableHeaps.front()];
        DX12DescriptorHandle allocation = heap.Allocate();

        if (heap.GetSize() == heap.GetCapacity())
        {
            // If the heap gets full remove it from the available list
            m_AvailableHeaps.pop();
        }

        ATOM_ENGINE_ASSERT(allocation.IsValid(), "Descriptor allocation failed!");
        return allocation;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12DescriptorAllocator::ReleaseDescriptors()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        for (u32 i = 0 ; i < m_HeapPool.size(); i++)
        {
            m_HeapPool[i]->ReleaseDescriptors();

            // If space has been freed in the heap, add it to the available heaps
            if (m_HeapPool[i]->GetSize() < m_HeapPool[i]->GetCapacity())
            {
                m_AvailableHeaps.push(i);
            }
        }
    }
}

#endif // ATOM_PLATFORM_WINDOWS
