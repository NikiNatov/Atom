#include "atompch.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Core/Application.h"

#include "CommandQueue.h"
#include "CommandBuffer.h"
#include "Device.h"
#include "Renderer.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    CommandQueue::CommandQueue(CommandQueueType type, const char* debugName)
        : m_Type(type)
    {
        auto d3dDevice = Device::Get().GetD3DDevice();

        // Create command queue
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.NodeMask = 0;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Type = Utils::AtomCommandQueueTypeToD3D12(m_Type);
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

        DXCall(d3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_D3DCommandQueue)));

        // Create fence
        DXCall(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_D3DFence)));

#if defined (ATOM_DEBUG)
        String name = debugName;
        DXCall(m_D3DCommandQueue->SetName(STRING_TO_WSTRING(name).c_str()));
#endif

        m_CmdBufferProcessingThread = std::thread(&CommandQueue::ProcessInFlightCommandBuffers, this);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    CommandQueue::~CommandQueue()
    {
        m_ProcessInFlightCmdBuffers = false;
        m_CmdBufferProcessingThread.join();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u64 CommandQueue::Signal()
    {
        DXCall(m_D3DCommandQueue->Signal(m_D3DFence.Get(), ++m_FenceValue));
        return m_FenceValue;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandQueue::WaitForFenceValue(u64 value)
    {
        if (m_D3DFence->GetCompletedValue() < value)
        {
            if (HANDLE fenceEvent = CreateEvent(0, false, false, 0))
            {
                // If the fence value is not reached pause the current thread
                DXCall(m_D3DFence->SetEventOnCompletion(value, fenceEvent));
                WaitForSingleObject(fenceEvent, INFINITE);
                CloseHandle(fenceEvent);
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandQueue::Flush()
    {
        std::unique_lock<std::mutex> lock(m_CmdBufferProcessingMutex);
        m_CmdBufferProcessingCV.wait(lock, [this] { return m_InFlightCmdBuffers.Empty(); });

        u64 fenceValueToWait = Signal();
        WaitForFenceValue(fenceValueToWait);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandQueue::WaitForQueue(const CommandQueue* queue)
    {
        m_D3DCommandQueue->Wait(queue->m_D3DFence.Get(), queue->m_FenceValue);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u64 CommandQueue::ExecuteCommandList(const Ref<CommandBuffer>& commandBuffer)
    {
        return ExecuteCommandLists({ commandBuffer });
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u64 CommandQueue::ExecuteCommandLists(const Vector<Ref<CommandBuffer>>& commandBuffers)
    {
        Vector<ID3D12CommandList*> commandListArray;

        for (auto& buffer : commandBuffers)
        {
            commandListArray.push_back(buffer->GetPendingCommandList().Get());
            commandListArray.push_back(buffer->GetCommandList().Get());
        }

        m_D3DCommandQueue->ExecuteCommandLists(commandListArray.size(), commandListArray.data());
        u64 fenceValue = Signal();

        for (auto& buffer : commandBuffers)
        {
            m_InFlightCmdBuffers.Emplace(buffer, fenceValue);
        }

        return fenceValue;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<CommandBuffer> CommandQueue::GetCommandBuffer()
    {
        if (!m_AvailableCmdBuffers.Empty())
        {
            return m_AvailableCmdBuffers.Pop();
        }

        String cmdBufferName;

        switch (m_Type)
        {
            case CommandQueueType::Graphics: cmdBufferName = "CmdBuffer(Graphics)"; break;
            case CommandQueueType::Compute:  cmdBufferName = "CmdBuffer(Compute)"; break;
            case CommandQueueType::Copy:     cmdBufferName = "CmdBuffer(Copy)"; break;
        }

        return CreateRef<CommandBuffer>(m_Type, cmdBufferName.c_str());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandQueue::ProcessInFlightCommandBuffers()
    {
        std::unique_lock<std::mutex> lock(m_CmdBufferProcessingMutex, std::defer_lock);

        while (m_ProcessInFlightCmdBuffers)
        {
            lock.lock();

            while (!m_InFlightCmdBuffers.Empty())
            {
                CommandBufferEntry entry = m_InFlightCmdBuffers.Pop();

                WaitForFenceValue(entry.FenceValue);
                m_AvailableCmdBuffers.Push(entry.CmdBuffer);
            }

            lock.unlock();
            m_CmdBufferProcessingCV.notify_one();

            std::this_thread::yield();
        }
    }
}
