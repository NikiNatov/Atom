#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"
#include "Texture.h"

#include <glm/glm.hpp>

namespace Atom
{
    class TextureViewRT;
    class TextureViewDS;

    enum AttachmentPoint : u8
    {
        Color0,
        Color1,
        Color2,
        Color3,
        Color4,
        Color5,
        Color6,
        Color7,
        DepthStencil,
        NumColorAttachments = DepthStencil,
        NumAttachments
    };

    struct FramebufferAttachment
    {
        TextureFormat Format;
        TextureFilter Filter;
        TextureWrap   Wrap;

        FramebufferAttachment()
            : Format(TextureFormat::None), Filter(TextureFilter::None), Wrap(TextureWrap::None)
        {}

        FramebufferAttachment(TextureFormat format, TextureFilter filter = TextureFilter::Linear, TextureWrap wrap = TextureWrap::Clamp)
            : Format(format), Filter(filter), Wrap(wrap)
        {}
    };

    struct FramebufferDescription
    {
        u32                   Width = 0;
        u32                   Height = 0;
        bool                  SwapChainFrameBuffer = false;
        glm::vec4             ClearColor { 0 };
        FramebufferAttachment Attachments[AttachmentPoint::NumAttachments];
        Ref<Texture>          OverrideAttachments[AttachmentPoint::NumAttachments];
    };

    class Framebuffer
    {
    public:
        Framebuffer(const FramebufferDescription& description);
        ~Framebuffer();

        void Resize(u32 width, u32 height);
        u32 GetWidth() const;
        u32 GetHeight() const;
        const glm::vec4& GetClearColor() const;
        bool IsSwapChainTarget() const;
        const Texture* GetAttachment(AttachmentPoint attachment) const;
        const TextureViewRT* GetRTV(AttachmentPoint attachment) const;
        const TextureViewDS* GetDSV() const;

        const D3D12_VIEWPORT& GetViewport() const;
        const D3D12_RECT& GetScissorRect() const;
    private:
        void Invalidate();
    private:
        FramebufferDescription m_Description;
        D3D12_VIEWPORT         m_Viewport{};
        D3D12_RECT             m_ScissorRect{};
        Ref<Texture>           m_Attachments[AttachmentPoint::NumAttachments]{ nullptr };
        Ref<TextureViewRT>     m_ColorAttachmentRTVs[AttachmentPoint::NumColorAttachments]{ nullptr };
        Ref<TextureViewDS>     m_DepthAttachmentDSV = nullptr;
    };
}