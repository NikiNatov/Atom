#include "atompch.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DX12CommandQueue.h"
#include "DX12CommandBuffer.h"
#include "DX12Device.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    DX12CommandQueue::DX12CommandQueue(Device& device, CommandQueueType type)
        : m_Type(type)
    {
        auto d3dDevice = device.As<DX12Device>()->GetD3DDevice();

        // Create command queue
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.NodeMask = 0;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Type = Utils::AtomCommandQueueTypeToD3D12(m_Type);
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
        COMSafeRelease(m_D3DCommandQueue);
        COMSafeRelease(m_D3DFence);

        if (m_FenceEvent)
        {
            CloseHandle(m_FenceEvent);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u64 DX12CommandQueue::Signal()
    {
        DXCall(m_D3DCommandQueue->Signal(m_D3DFence, ++m_FenceValue));
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
    u64 DX12CommandQueue::ExecuteCommandList(const Ref<CommandBuffer>& commandBuffer)
    {
        auto d3dCommandList = commandBuffer->As<DX12CommandBuffer>()->GetCommandList();
        ID3D12CommandList* commandListArray[] = { d3dCommandList };
        m_D3DCommandQueue->ExecuteCommandLists(1, commandListArray);
        return Signal();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u64 DX12CommandQueue::ExecuteCommandLists(const Vector<Ref<CommandBuffer>>& commandBuffers)
    {
        Vector<ID3D12CommandList*> commandListArray;
        commandListArray.reserve(commandBuffers.size());

        for (auto list : commandBuffers)
        {
            auto d3dCommandList = list->As<DX12CommandBuffer>()->GetCommandList();
            commandListArray.push_back(d3dCommandList);
        }

        m_D3DCommandQueue->ExecuteCommandLists(commandBuffers.size(), commandListArray.data());
        return Signal();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    CommandQueueType DX12CommandQueue::GetQueueType() const
    {
        return m_Type;
    }
}
#endif // ATOM_PLATFORM_WINDOWS