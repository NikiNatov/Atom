#include "atompch.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DX12CommandQueue.h"
#include "DX12Device.h"
#include "DX12GraphicsCommandList.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    DX12CommandQueue::DX12CommandQueue(const Device* device, const CommandQueueDesc& description)
        : m_Device(device), m_Description(description)
    {
        auto d3dDevice = m_Device->As<DX12Device>()->GetD3DDevice();

        // Create command queue
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.NodeMask = 0;
        desc.Priority = Utils::AtomCommandQueuePriorityToD3D12(m_Description.Priority);
        desc.Type = Utils::AtomCommandListTypeToD3D12(m_Description.Type);
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

        DXCall(d3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_D3DCommandQueue)));

        // Create fence
        DXCall(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_D3DFence)));

        // Create event
        m_FenceEvent = CreateEvent(0, false, false, 0);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12CommandQueue::~DX12CommandQueue()
    {
        if (m_FenceEvent)
            CloseHandle(m_FenceEvent);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u64 DX12CommandQueue::Signal()
    {
        DXCall(m_D3DCommandQueue->Signal(m_D3DFence.Get(), ++m_FenceValue));
        return m_FenceValue;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandQueue::WaitForFenceValue(u64 value)
    {
        if (m_D3DFence->GetCompletedValue() < value)
        {
            // If the fence value is not reached pause the current thread
            DXCall(m_D3DFence->SetEventOnCompletion(value, m_FenceEvent));
            WaitForSingleObject(m_FenceEvent, INFINITE);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandQueue::Flush()
    {
        u64 fenceValueToWait = Signal();
        WaitForFenceValue(fenceValueToWait);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandQueue::ExecuteCommandList(const GraphicsCommandList* commandList)
    {
        auto d3dCommandList = commandList->As<DX12GraphicsCommandList>()->GetD3DGraphicsCommandList();
        ID3D12CommandList* commandListArray[] = { d3dCommandList.Get() };
        m_D3DCommandQueue->ExecuteCommandLists(1, commandListArray);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandQueue::ExecuteCommandLists(const Vector<GraphicsCommandList*>& commandLists)
    {
        Vector<ID3D12CommandList*> commandListArray;
        commandListArray.reserve(commandLists.size());

        for (auto list : commandLists)
        {
            auto d3dCommandList = list->As<DX12GraphicsCommandList>()->GetD3DGraphicsCommandList();
            commandListArray.push_back(d3dCommandList.Get());
        }

        m_D3DCommandQueue->ExecuteCommandLists(commandLists.size(), commandListArray.data());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    CommandListType DX12CommandQueue::GetQueueType() const
    {
        return m_Description.Type;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    CommandQueuePriority DX12CommandQueue::GetQueuePriority() const
    {
        return m_Description.Priority;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Device* DX12CommandQueue::GetCreationDevice() const
    {
        return m_Device;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u64 DX12CommandQueue::GetCurrentFenceValue() const
    {
        return m_FenceValue;
    }
}
#endif // ATOM_PLATFORM_WINDOWS