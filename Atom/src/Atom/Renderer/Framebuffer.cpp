#include "atompch.h"

#include "Atom/Core/Application.h"

#include "Framebuffer.h"
#include "Device.h"
#include "SwapChain.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Framebuffer::Framebuffer(const FramebufferDescription& description, const char* name)
        : m_Description(description), m_Name(name)
    {
        if (!m_Description.SwapChainFrameBuffer)
        {
            Invalidate();
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Framebuffer::~Framebuffer()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Framebuffer::Resize(u32 width, u32 height)
    {
        if (!m_Description.SwapChainFrameBuffer)
        {
            if (m_Description.Width != width || m_Description.Height != height)
            {
                m_Description.Width = width;
                m_Description.Height = height;

                Invalidate();
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const String& Framebuffer::GetName() const
    {
        return m_Name;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Framebuffer::GetWidth() const
    {
        if (m_Description.SwapChainFrameBuffer)
        {
            return Application::Get().GetWindow().GetSwapChain()->GetWidth();
        }

        return m_Description.Width;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Framebuffer::GetHeight() const
    {
        if (m_Description.SwapChainFrameBuffer)
        {
            return Application::Get().GetWindow().GetSwapChain()->GetHeight();
        }

        return m_Description.Height;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const ClearValue& Framebuffer::GetClearValue(AttachmentPoint attachment) const
    {
        return m_Description.Attachments[attachment].ClearVal;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool Framebuffer::IsSwapChainTarget() const
    {
        return m_Description.SwapChainFrameBuffer;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<RenderSurface> Framebuffer::GetAttachment(AttachmentPoint attachment) const
    {
        if (m_Description.SwapChainFrameBuffer)
        {
            return attachment == AttachmentPoint::Color0 ? Application::Get().GetWindow().GetSwapChain()->GetBackBuffer() : nullptr;
        }

        return m_Attachments[attachment];
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const D3D12_VIEWPORT& Framebuffer::GetViewport() const
    {
        if (m_Description.SwapChainFrameBuffer)
        {
            return Application::Get().GetWindow().GetSwapChain()->GetViewport();
        }

        return m_Viewport;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const D3D12_RECT& Framebuffer::GetScissorRect() const
    {
        if (m_Description.SwapChainFrameBuffer)
        {
            return Application::Get().GetWindow().GetSwapChain()->GetScissorRect();
        }

        return m_ScissorRect;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Framebuffer::Invalidate()
    {
        for (u32 i = 0; i < AttachmentPoint::NumAttachments; i++)
            m_Attachments[i] = nullptr;

        for (u32 i = 0; i < AttachmentPoint::NumAttachments; i++)
        {
            if (m_Description.Attachments[i].Format != TextureFormat::None)
            {
                bool isDepthAttachment = i == AttachmentPoint::Depth;

                TextureDescription attachmentDesc;
                attachmentDesc.Format = m_Description.Attachments[i].Format;
                attachmentDesc.Width = m_Description.Width;
                attachmentDesc.Height = m_Description.Height;
                attachmentDesc.MipLevels = 1;
                attachmentDesc.Flags = (isDepthAttachment ? TextureFlags::DepthStencil : TextureFlags::RenderTarget) | TextureFlags::ShaderResource;
                attachmentDesc.ClearValue = m_Description.Attachments[i].ClearVal;

                Ref<Texture> texture = CreateRef<Texture>(attachmentDesc, fmt::format("{}_{}", m_Name.c_str(), isDepthAttachment ? "DepthAttachment" : "ColorAttachment").c_str());
                m_Attachments[i] = CreateRef<RenderSurface>(texture, 0, 0);
            }
        }

        // Recreate viewport
        m_Viewport.Width = (f32)m_Description.Width;
        m_Viewport.Height = (f32)m_Description.Height;
        m_Viewport.TopLeftX = 0.0f;
        m_Viewport.TopLeftY = 0.0f;
        m_Viewport.MinDepth = 0.0f;
        m_Viewport.MaxDepth = 1.0f;

        // Recreate scissor rect
        m_ScissorRect = { 0, 0, (s32)m_Description.Width, (s32)m_Description.Height };
    }
}
