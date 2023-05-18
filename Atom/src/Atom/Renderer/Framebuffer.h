#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"
#include "Atom/Renderer/RenderSurface.h"

#include <glm/glm.hpp>

namespace Atom
{
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
        Depth,
        NumColorAttachments = Depth,
        NumAttachments
    };

    struct FramebufferAttachment
    {
        TextureFormat Format;
        TextureFilter Filter;
        TextureWrap   Wrap;
        ClearValue    ClearVal;

        FramebufferAttachment()
            : Format(TextureFormat::None), Filter(TextureFilter::None), Wrap(TextureWrap::None)
        {}

        FramebufferAttachment(TextureFormat format, ClearValue clearValue, TextureFilter filter = TextureFilter::Linear, TextureWrap wrap = TextureWrap::Clamp)
            : Format(format), Filter(filter), Wrap(wrap), ClearVal(clearValue)
        {}
    };

    struct FramebufferDescription
    {
        u32                   Width = 0;
        u32                   Height = 0;
        bool                  SwapChainFrameBuffer = false;
        FramebufferAttachment Attachments[AttachmentPoint::NumAttachments];
        Ref<Texture>          OverrideAttachments[AttachmentPoint::NumAttachments];
    };

    class Framebuffer
    {
    public:
        Framebuffer(const FramebufferDescription& description, const char* name = "Unnamed Framebuffer");
        ~Framebuffer();

        void Resize(u32 width, u32 height);
        const String& GetName() const;
        u32 GetWidth() const;
        u32 GetHeight() const;
        const ClearValue& GetClearValue(AttachmentPoint attachment) const;
        bool IsSwapChainTarget() const;
        Ref<RenderSurface> GetAttachment(AttachmentPoint attachment) const;

        const D3D12_VIEWPORT& GetViewport() const;
        const D3D12_RECT& GetScissorRect() const;
    private:
        void Invalidate();
    private:
        String                 m_Name;
        FramebufferDescription m_Description;
        D3D12_VIEWPORT         m_Viewport{};
        D3D12_RECT             m_ScissorRect{};
        Ref<RenderSurface>     m_Attachments[AttachmentPoint::NumAttachments]{ nullptr };
    };
}