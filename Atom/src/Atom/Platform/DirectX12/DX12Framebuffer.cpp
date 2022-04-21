#include "atompch.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DX12Framebuffer.h"
#include "DX12Device.h"
#include "DX12SwapChain.h"

#include "Atom/Core/Application.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    DX12Framebuffer::DX12Framebuffer(const FramebufferDescription& description)
        : m_Description(description)
    {
        if (!m_Description.SwapChainFrameBuffer)
        {
            Invalidate();
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12Framebuffer::~DX12Framebuffer()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12Framebuffer::Resize(u32 width, u32 height)
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
    u32 DX12Framebuffer::GetWidth() const
    {
        if (m_Description.SwapChainFrameBuffer)
        {
            return Application::Get().GetWindow().GetSwapChain().GetWidth();
        }

        return m_Description.Width;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12Framebuffer::GetHeight() const
    {
        if (m_Description.SwapChainFrameBuffer)
        {
            return Application::Get().GetWindow().GetSwapChain().GetHeight();
        }

        return m_Description.Height;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const f32* DX12Framebuffer::GetClearColor() const
    {
        return m_Description.ClearColor;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool DX12Framebuffer::IsSwapChainTarget() const
    {
        return m_Description.SwapChainFrameBuffer;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Texture* DX12Framebuffer::GetAttachment(AttachmentPoint attachment) const
    {
        if (m_Description.SwapChainFrameBuffer)
        {
            return attachment == AttachmentPoint::Color0 ? Application::Get().GetWindow().GetSwapChain().GetBackBuffer() : nullptr;
        }

        return m_Attachments[attachment].get();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const D3D12_VIEWPORT& DX12Framebuffer::GetViewport() const
    {
        if (m_Description.SwapChainFrameBuffer)
        {
            return Application::Get().GetWindow().GetSwapChain().As<DX12SwapChain>()->GetViewport();
        }

        return m_Viewport;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const D3D12_RECT& DX12Framebuffer::GetScissorRect() const
    {
        if (m_Description.SwapChainFrameBuffer)
        {
            return Application::Get().GetWindow().GetSwapChain().As<DX12SwapChain>()->GetScissorRect();
        }

        return m_ScissorRect;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const TextureViewRT* DX12Framebuffer::GetRTV(AttachmentPoint attachment) const
    {
        ATOM_ENGINE_ASSERT(attachment != AttachmentPoint::DepthStencil, "Depth attachment does not have RTV!");

        if (m_Description.SwapChainFrameBuffer)
        {
            return attachment == AttachmentPoint::Color0 ? Application::Get().GetWindow().GetSwapChain().GetBackBufferRTV() : nullptr;
        }

        return m_ColorAttachmentRTVs[attachment].get();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const TextureViewDS* DX12Framebuffer::GetDSV() const
    {
        if (m_Description.SwapChainFrameBuffer)
        {
            return nullptr;
        }

        return m_DepthAttachmentDSV.get();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12Framebuffer::Invalidate()
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

                m_Attachments[i] = Texture::CreateTexture2D(attachmentDesc, isDepthAttachment ? "DepthBuffer" : fmt::format("ColorAttachment[{}]", i).c_str());

                if (isDepthAttachment)
                {
                    m_DepthAttachmentDSV = TextureViewDS::Create(m_Attachments[i]);
                }
                else
                {
                    m_ColorAttachmentRTVs[i] = TextureViewRT::Create(m_Attachments[i]);
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

#endif // ATOM_PLATFORM_WINDOWS