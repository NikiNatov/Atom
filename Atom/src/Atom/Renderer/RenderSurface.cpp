#include "atompch.h"
#include "RenderSurface.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Renderer/Device.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurface::RenderSurface(const Ref<Texture>& texture, u32 mipIndex, u32 sliceIndex)
        : m_Texture(texture)
    {
        ATOM_ENGINE_ASSERT(m_Texture);

        // Create views
        TextureViewDescription viewDesc;
        viewDesc.FirstMip = mipIndex == TextureView::AllMips ? 0 : mipIndex;
        viewDesc.MipLevels = mipIndex == TextureView::AllMips ? m_Texture->GetMipLevels() : 1;
        viewDesc.FirstSlice = sliceIndex == TextureView::AllSlices ? 0 : sliceIndex;
        viewDesc.ArraySize = sliceIndex == TextureView::AllSlices ? m_Texture->GetArraySize() : 1;

        if (IsSet(m_Texture->GetFlags() & TextureFlags::RenderTarget))
            m_RTV = CreateScope<TextureViewRT>(this, viewDesc, !IsSet(m_Texture->GetFlags() & TextureFlags::SwapChainBuffer));
        if (IsSet(m_Texture->GetFlags() & TextureFlags::DepthStencil))
            m_DSV = CreateScope<TextureViewDS>(this, viewDesc, !IsSet(m_Texture->GetFlags() & TextureFlags::SwapChainBuffer));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurface::~RenderSurface()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurface::RenderSurface(RenderSurface&& rhs) noexcept
        : m_Texture(std::move(rhs.m_Texture)), m_RTV(std::move(rhs.m_RTV)), m_DSV(std::move(rhs.m_DSV))
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurface& RenderSurface::operator=(RenderSurface&& rhs) noexcept
    {
        if (this != &rhs)
        {
            m_Texture = std::move(rhs.m_Texture);
            m_RTV = std::move(rhs.m_RTV);
            m_DSV = std::move(rhs.m_DSV);
        }

        return *this;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureFormat RenderSurface::GetFormat() const
    {
        return m_Texture->GetFormat();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 RenderSurface::GetWidth() const
    {
        return m_Texture->GetWidth();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 RenderSurface::GetHeight() const
    {
        return m_Texture->GetHeight();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 RenderSurface::GetDepth() const
    {
        return m_Texture->GetDepth();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 RenderSurface::GetArraySize() const
    {
        return m_Texture->GetArraySize();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureFlags RenderSurface::GetFlags() const
    {
        return m_Texture->GetFlags();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const ClearValue& RenderSurface::GetClearValue() const
    {
        return m_Texture->GetClearValue();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Ref<Texture> RenderSurface::GetTexture() const
    {
        return m_Texture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool RenderSurface::IsSwapChainBuffer() const
    {
        return IsSet(m_Texture->GetFlags() & TextureFlags::SwapChainBuffer);
    }
}
