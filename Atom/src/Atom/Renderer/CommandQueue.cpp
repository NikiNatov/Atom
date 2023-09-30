#include "atompch.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Core/Application.h"

#include "CommandQueue.h"
#include "CommandBuffer.h"
#include "Device.h"
#include "Renderer.h"
#include "Fence.h"

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

        // Create cmd buffer processing fence
        m_CmdBufferProcessingFence = CreateRef<Fence>("Command Buffer Processing Fence");

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
    void CommandQueue::SignalFence(const Ref<Fence>& fence, u64 value)
    {
        DXCall(m_D3DCommandQueue->Signal(fence->GetD3DFence().Get(), value));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandQueue::Flush()
    {
        std::unique_lock<std::mutex> lock(m_CmdBufferProcessingMutex);
        m_CmdBufferProcessingCV.wait(lock, [this] { return m_InFlightCmdBuffers.Empty(); });

        Ref<Fence> fence = CreateRef<Fence>();
        u64 fenceValueToWait = fence->IncrementTargetValue();
        SignalFence(fence, fenceValueToWait);
        fence->WaitForValueCPU(fenceValueToWait);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandQueue::WaitFence(const Ref<Fence>& fence, u64 value)
    {
        m_D3DCommandQueue->Wait(fence->GetD3DFence().Get(), value);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandQueue::ExecuteCommandList(const Ref<CommandBuffer>& commandBuffer)
    {
        ExecuteCommandLists({ commandBuffer });
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandQueue::ExecuteCommandLists(const Vector<Ref<CommandBuffer>>& commandBuffers)
    {
        if (commandBuffers.empty())
            return;

        Vector<ID3D12CommandList*> commandListArray;

        for (auto& buffer : commandBuffers)
        {
            commandListArray.push_back(buffer->GetPendingCommandList().Get());
            commandListArray.push_back(buffer->GetCommandList().Get());
        }

        m_D3DCommandQueue->ExecuteCommandLists(commandListArray.size(), commandListArray.data());
        u64 fenceValue = m_CmdBufferProcessingFence->IncrementTargetValue();
        SignalFence(m_CmdBufferProcessingFence, fenceValue);

        for (auto& buffer : commandBuffers)
        {
            m_InFlightCmdBuffers.Emplace(buffer, fenceValue);
        }
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

                m_CmdBufferProcessingFence->WaitForValueCPU(entry.FenceValue);
                m_AvailableCmdBuffers.Push(entry.CmdBuffer);
            }

            lock.unlock();
            m_CmdBufferProcessingCV.notify_one();

            std::this_thread::yield();
        }
    }
}
