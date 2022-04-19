#pragma once

#include "Atom/Renderer/API/Framebuffer.h"
#include "Atom/Renderer/API/TextureView.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"

namespace Atom
{
    class DX12Framebuffer : public Framebuffer
    {
    public:
        DX12Framebuffer(const FramebufferDescription& description);
        ~DX12Framebuffer();

        virtual void Resize(u32 width, u32 height) override;
        virtual u32 GetWidth() const override;
        virtual u32 GetHeight() const override;
        virtual const f32* GetClearColor() const override;
        virtual bool IsSwapChainTarget() const override;
        virtual const Ref<Texture>& GetAttachmnt(AttachmentPoint attachment) const override;
        virtual const Ref<TextureViewRT>& GetRTV(AttachmentPoint attachment) const override;
        virtual const Ref<TextureViewDS>& GetDSV() const override;

        const D3D12_VIEWPORT& GetViewport() const;
        const D3D12_RECT& GetScissorRect() const;
    private:
        void Invalidate();
    private:
        FramebufferDescription     m_Description;
        D3D12_VIEWPORT             m_Viewport{};
        D3D12_RECT                 m_ScissorRect{};
        Ref<Texture>               m_Attachments[AttachmentPoint::NumAttachments]{ nullptr };
        Ref<TextureViewRT>         m_ColorAttachmentRTVs[AttachmentPoint::NumColorAttachments]{ nullptr };
        Ref<TextureViewDS>         m_DepthAttachmentDSV = nullptr;
    };

}

#endif // ATOM_PLATFORM_WINDOWS