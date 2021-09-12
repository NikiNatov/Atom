#pragma once

#include "Atom/Renderer/CommandQueue.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"

namespace Atom
{
    class DX12CommandQueue : public CommandQueue
    {
    public:
        DX12CommandQueue(const Device* device, const CommandQueueDesc& description);
        ~DX12CommandQueue();

        virtual u64 Signal() override;
        virtual void WaitForFenceValue(u64 value) override;
        virtual void Flush() override;

        virtual void ExecuteCommandList(const GraphicsCommandList* commandList) override;
        virtual void ExecuteCommandLists(const Vector<GraphicsCommandList*>& commandLists) override;

        virtual CommandListType GetQueueType() const override;
        virtual CommandQueuePriority GetQueuePriority() const override;
        virtual const Device* GetCreationDevice() const override;
        virtual u64 GetCurrentFenceValue() const override;

        inline wrl::ComPtr<ID3D12CommandQueue> GetD3DCommandQueue() const { return m_D3DCommandQueue; }
        inline wrl::ComPtr<ID3D12Fence1> GetD3DFence() const { return m_D3DFence; }
    private:
        const Device*                   m_Device;
        CommandQueueDesc                m_Description;
        wrl::ComPtr<ID3D12CommandQueue> m_D3DCommandQueue;
        wrl::ComPtr<ID3D12Fence1>       m_D3DFence;
        HANDLE                          m_FenceEvent;
        u64                             m_FenceValue = 0;
    };
}
#endif // ATOM_PLATFORM_WINDOWS