#pragma once

#include "Atom/Renderer/API/CommandBuffer.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"
#include "DX12ResourceStateTracker.h"

namespace Atom
{
    class DX12CommandBuffer : public CommandBuffer
    {
    public:
        DX12CommandBuffer(const char* debugName = "Unnamed Command Buffer");
        ~DX12CommandBuffer();

        virtual void Begin() override;
        virtual void BeginRenderPass(const Framebuffer* framebuffer, bool clear = false) override;
        virtual void EndRenderPass(const Framebuffer* framebuffer) override;
        virtual void SetGraphicsPipeline(const GraphicsPipeline* pipeline) override;
        virtual void Draw(u32 count) override;
        virtual void End() override;

        inline ComPtr<ID3D12GraphicsCommandList6> GetCommandList() const { return m_CommandList; }
        inline ComPtr<ID3D12GraphicsCommandList6> GetPendingCommandList() const { return m_PendingCommandList; }
    private:
        Vector<ComPtr<ID3D12CommandAllocator>> m_Allocators;
        ComPtr<ID3D12GraphicsCommandList6>     m_CommandList;
        Vector<ComPtr<ID3D12CommandAllocator>> m_PendingAllocators;
        ComPtr<ID3D12GraphicsCommandList6>     m_PendingCommandList;
        DX12ResourceStateTracker               m_ResourceStateTracker;
    };
}

#endif // ATOM_PLATFORM_WINDOWS