#pragma once

#include "Atom/Renderer/API/CommandQueue.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"

namespace Atom
{
    class CommandBuffer;

    class DX12CommandQueue : public CommandQueue
    {
    public:
        DX12CommandQueue(CommandQueueType type);
        ~DX12CommandQueue();

        virtual u64 Signal() override;
        virtual void WaitForFenceValue(u64 value) override;
        virtual void Flush() override;
        virtual u64 ExecuteCommandList(const Ref<CommandBuffer>& commandBuffer) override;
        virtual u64 ExecuteCommandLists(const Vector<Ref<CommandBuffer>>& commandBuffers) override;
        virtual CommandQueueType GetQueueType() const override;

        inline ComPtr<ID3D12CommandQueue> GetD3DCommandQueue() const { return m_D3DCommandQueue; }
        inline ComPtr<ID3D12Fence1> GetD3DFence() const { return m_D3DFence; }
    private:
        ComPtr<ID3D12CommandQueue> m_D3DCommandQueue;
        CommandQueueType           m_Type;
        ComPtr<ID3D12Fence1>       m_D3DFence;
        HANDLE                     m_FenceEvent;
        u64                        m_FenceValue = 0;
        Vector<ID3D12CommandList*> m_QueuedCommandLists;
    };
}
#endif // ATOM_PLATFORM_WINDOWS
