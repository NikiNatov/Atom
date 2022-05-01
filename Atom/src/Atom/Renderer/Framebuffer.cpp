#include "atompch.h"

#include "Atom/Core/Application.h"

#include "Framebuffer.h"
#include "Device.h"
#include "SwapChain.h"
#include "TextureView.h"

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
    const Texture* Framebuffer::GetAttachment(AttachmentPoint attachment) const
    {
        if (m_Description.SwapChainFrameBuffer)
        {
            return attachment == AttachmentPoint::Color0 ? Application::Get().GetWindow().GetSwapChain()->GetBackBuffer() : nullptr;
        }

        return m_Attachments[attachment].get();
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
    const TextureViewRT* Framebuffer::GetRTV(AttachmentPoint attachment) const
    {
        ATOM_ENGINE_ASSERT(attachment != AttachmentPoint::DepthStencil, "Depth attachment does not have RTV!");

        if (m_Description.SwapChainFrameBuffer)
        {
            return attachment == AttachmentPoint::Color0 ? Application::Get().GetWindow().GetSwapChain()->GetBackBufferRTV() : nullptr;
        }

        return m_ColorAttachmentRTVs[attachment].get();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const TextureViewDS* Framebuffer::GetDSV() const
    {
        if (m_Description.SwapChainFrameBuffer)
        {
            return nullptr;
        }

        return m_DepthAttachmentDSV.get();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Framebuffer::Invalidate()
    {
        for (u32 i = 0; i < AttachmentPoint::NumAttachments; i++)
            m_Attachments[i] = nullptr;

        for (u32 i = 0; i < AttachmentPoint::NumColorAttachments; i++)
            m_ColorAttachmentRTVs[i] = nullptr;

        m_DepthAttachmentDSV = nullptr;

        for (u32 i = 0; i < AttachmentPoint::NumAttachments; i++)
        {
            if (m_Description.Attachments[i].Format != TextureFormat::None)
            {
                bool isDepthAttachment = i == AttachmentPoint::DepthStencil;

                TextureDescription attachmentDesc;
                attachmentDesc.Format = m_Description.Attachments[i].Format;
                attachmentDesc.Width = m_Description.Width;
                attachmentDesc.Height = m_Description.Height;
                attachmentDesc.MipLevels = 1;
                attachmentDesc.Filter = m_Description.Attachments[i].Filter;
                attachmentDesc.Wrap = m_Description.Attachments[i].Wrap;
                attachmentDesc.UsageFlags = TextureUsage::ShaderResource | (isDepthAttachment ? TextureUsage::DepthBuffer : TextureUsage::RenderTarget);

                m_Attachments[i] = CreateRef<Texture>(TextureType::Texture2D, attachmentDesc, isDepthAttachment ? "DepthBuffer" : fmt::format("ColorAttachment[{}]", i).c_str());

                if (isDepthAttachment)
                {
                    m_DepthAttachmentDSV = CreateRef<TextureViewDS>(m_Attachments[i]);
                }
                else
                {
                    m_ColorAttachmentRTVs[i] = CreateRef<TextureViewRT>(m_Attachments[i]);
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