#include "atompch.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"

#include "DescriptorHeap.h"
#include "Device.h"
#include "Renderer.h"

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

    // -----------------------------------------------------------------------------------------------------------------------------
    CPUDescriptorHeap::CPUDescriptorHeap(DescriptorHeapType type, u32 capacity, const char* debugName)
        : DescriptorHeap(type, capacity, false, debugName)
    {
        // Initialize the free slot indices
        for (u32 i = 0; i < m_Capacity; i++)
        {
            m_FreeSlots.push(i);
        }

        // Create arrays for deferred release descriptors for each frame in flight
        m_DeferredReleaseDescriptors.resize(Renderer::GetFramesInFlight());
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
            u32 descriptorIndex = (descriptor.ptr - m_CPUStartHandle.ptr) % m_DescriptorSize;

            if (deferredRelease)
            {
                m_DeferredReleaseDescriptors[Renderer::GetCurrentFrameIndex()].push_back(descriptorIndex);
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
}