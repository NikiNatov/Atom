#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

#include "Atom/Core/DataStructures/ThreadSafeQueue.h"

namespace Atom
{
    class CommandBuffer;

    enum class CommandQueueType
    {
        Graphics,
        Compute,
        Copy,
        TypeCount = Copy
    };

    class CommandQueue
    {
    public:
        CommandQueue(CommandQueueType type, const char* debugName = "Unnamed Command Queue");
        ~CommandQueue();

        u64 Signal();
        void WaitForFenceValue(u64 value);
        void WaitForQueue(const CommandQueue* queue);
        void Flush();
        u64 ExecuteCommandList(const Ref<CommandBuffer>& commandBuffer);
        u64 ExecuteCommandLists(const Vector<Ref<CommandBuffer>>& commandBuffers);
        Ref<CommandBuffer> GetCommandBuffer();
        inline CommandQueueType GetQueueType() const { return m_Type; }
        inline ComPtr<ID3D12CommandQueue> GetD3DCommandQueue() const { return m_D3DCommandQueue; }
        inline ComPtr<ID3D12Fence1> GetD3DFence() const { return m_D3DFence; }
    private:
        void ProcessInFlightCommandBuffers();
    private:
        ComPtr<ID3D12CommandQueue> m_D3DCommandQueue;
        CommandQueueType           m_Type;
        ComPtr<ID3D12Fence1>       m_D3DFence;
        HANDLE                     m_FenceEvent;
        u64                        m_FenceValue = 0;

        struct CommandBufferEntry
        {
            Ref<CommandBuffer> CmdBuffer;
            u64                FenceValue;

            CommandBufferEntry(const Ref<CommandBuffer>& buffer, u64 fence)
                : CmdBuffer(buffer), FenceValue(fence) {}
        };

        ThreadSafeQueue<CommandBufferEntry> m_InFlightCmdBuffers;
        ThreadSafeQueue<Ref<CommandBuffer>> m_AvailableCmdBuffers;
        bool                                m_ProcessInFlightCmdBuffers = true;
        std::condition_variable             m_CmdBufferProcessingCV;
        std::mutex                          m_CmdBufferProcessingMutex;
        std::thread                         m_CmdBufferProcessingThread;

    };
}