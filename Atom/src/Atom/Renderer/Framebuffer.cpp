#include "atompch.h"

#include "Atom/Core/Application.h"

#include "Framebuffer.h"
#include "Device.h"
#include "SwapChain.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Framebuffer::Framebuffer(const FramebufferDescription& description)
        : m_Description(description)
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
    const glm::vec4& Framebuffer::GetClearColor() const
    {
        return m_Description.ClearColor;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool Framebuffer::IsSwapChainTarget() const
    {
        return m_Description.SwapChainFrameBuffer;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const RenderTexture2D* Framebuffer::GetColorAttachment(AttachmentPoint attachment) const
    {
        if (m_Description.SwapChainFrameBuffer)
        {
            return attachment == AttachmentPoint::Color0 ? Application::Get().GetWindow().GetSwapChain()->GetBackBuffer() : nullptr;
        }

        return m_ColorAttachments[attachment].get();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const DepthBuffer* Framebuffer::GetDepthAttachment() const
    {
        return m_DepthAttachment.get();
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
            m_ColorAttachments[i] = nullptr;

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
                attachmentDesc.Filter = m_Description.Attachments[i].Filter;
                attachmentDesc.Wrap = m_Description.Attachments[i].Wrap;
                attachmentDesc.UsageFlags = (isDepthAttachment ? TextureBindFlags::DepthStencil : TextureBindFlags::RenderTarget);

                if (isDepthAttachment)
                {
                    m_DepthAttachment = CreateRef<DepthBuffer>(attachmentDesc, "DepthBuffer");
                }
                else
                {
                    m_ColorAttachments[i] = CreateRef<RenderTexture2D>(attachmentDesc, fmt::format("ColorAttachment[{}]", i).c_str());
                }
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
