#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

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
        void Flush();
        u64 ExecuteCommandList(const CommandBuffer* commandBuffer);
        u64 ExecuteCommandLists(const Vector<const CommandBuffer*>& commandBuffers);
        inline CommandQueueType GetQueueType() const { return m_Type; }
        inline ComPtr<ID3D12CommandQueue> GetD3DCommandQueue() const { return m_D3DCommandQueue; }
        inline ComPtr<ID3D12Fence1> GetD3DFence() const { return m_D3DFence; }
    private:
        ComPtr<ID3D12CommandQueue> m_D3DCommandQueue;
        CommandQueueType           m_Type;
        ComPtr<ID3D12Fence1>       m_D3DFence;
        HANDLE                     m_FenceEvent;
        u64                        m_FenceValue = 0;
    };
}