#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

#include "Atom/Core/DataStructures/ThreadSafeQueue.h"

namespace Atom
{
    class CommandBuffer;
    class Fence;

    enum class CommandQueueType
    {
        Graphics,
        Compute,
        Copy,
        NumTypes
    };

    class CommandQueue
    {
    public:
        CommandQueue(CommandQueueType type, const char* debugName = "Unnamed Command Queue");
        ~CommandQueue();

        void SignalFence(const Ref<Fence>& fence, u64 value);
        void WaitFence(const Ref<Fence>& fence, u64 value);
        void Flush();
        void ExecuteCommandList(const Ref<CommandBuffer>& commandBuffer);
        void ExecuteCommandLists(const Vector<Ref<CommandBuffer>>& commandBuffers);
        Ref<CommandBuffer> GetCommandBuffer();
        inline CommandQueueType GetQueueType() const { return m_Type; }
        inline ComPtr<ID3D12CommandQueue> GetD3DCommandQueue() const { return m_D3DCommandQueue; }
    private:
        void ProcessInFlightCommandBuffers();
    private:
        ComPtr<ID3D12CommandQueue> m_D3DCommandQueue;
        CommandQueueType           m_Type;

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
        Ref<Fence>                          m_CmdBufferProcessingFence;

    };
}