#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Renderer.h"
#include "Texture.h"

namespace Atom
{
    class TextureViewRT;
    class TextureViewDS;
    class DX12Framebuffer;

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
        f32                   ClearColor[4]{ 0 };
        FramebufferAttachment Attachments[AttachmentPoint::NumAttachments];
        Ref<Texture>          OverrideAttachments[AttachmentPoint::NumAttachments];
    };

    class Framebuffer
    {
    public:
        virtual ~Framebuffer() = default;

        virtual void Resize(u32 width, u32 height) = 0;
        virtual u32 GetWidth() const = 0;
        virtual u32 GetHeight() const = 0;
        virtual const f32* GetClearColor() const = 0;
        virtual bool IsSwapChainTarget() const = 0;
        virtual const Ref<Texture>& GetAttachmnt(AttachmentPoint attachment) const = 0;
        virtual const Ref<TextureViewRT>& GetRTV(AttachmentPoint attachment) const = 0;
        virtual const Ref<TextureViewDS>& GetDSV() const = 0;

        IMPL_API_CAST(Framebuffer)

        static Ref<Framebuffer> Create(const FramebufferDescription& description);
    };
}