#pragma once

#include "Atom/Renderer/API/CommandBuffer.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"

namespace Atom
{
    class DX12CommandBuffer : public CommandBuffer
    {
    public:
        DX12CommandBuffer();
        ~DX12CommandBuffer();

        virtual void Begin() override;
        virtual void TransitionResource(const Ref<Texture>& texture, ResourceState beforeState, ResourceState afterState) override;
        virtual void BeginRenderPass(const Ref<Framebuffer>& framebuffer, bool clear = false) override;
        virtual void SetGraphicsPipeline(const Ref<GraphicsPipeline>& pipeline) override;
        virtual void Draw(u32 count) override;
        virtual void End() override;

        inline ComPtr<ID3D12GraphicsCommandList6> GetCommandList() const { return m_CommandList; }
    private:
        Vector<ComPtr<ID3D12CommandAllocator>> m_Allocators;
        ComPtr<ID3D12GraphicsCommandList6>     m_CommandList;
        Vector<u64>                            m_FenceValues;
    };
}

#endif // ATOM_PLATFORM_WINDOWS