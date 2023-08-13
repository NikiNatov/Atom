#include "atompch.h"
#include "RenderSurface.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Renderer/Device.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurface::RenderSurface(const TextureDescription& description, const char* debugName)
    {
        // Create texture resource
        m_Texture = CreateRef<Texture>(description, fmt::format("{}_Texture", debugName).c_str());

        // Create views
        TextureViewDescription viewDesc;
        viewDesc.FirstMip = 0;
        viewDesc.MipLevels = description.MipLevels;
        viewDesc.FirstSlice = 0;
        viewDesc.ArraySize = description.Type == TextureType::Texture3D ? description.Depth : description.ArraySize;

        if (IsSet(m_Texture->GetFlags() & TextureFlags::RenderTarget))
            m_RTV = CreateScope<TextureViewRT>(this, viewDesc, !IsSet(m_Texture->GetFlags() & TextureFlags::SwapChainBuffer));
        if (IsSet(m_Texture->GetFlags() & TextureFlags::DepthStencil))
            m_DSV = CreateScope<TextureViewDS>(this, viewDesc, !IsSet(m_Texture->GetFlags() & TextureFlags::SwapChainBuffer));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurface::RenderSurface(const RenderSurface& aliasedSurface, u32 mipIndex, u32 sliceIndex)
    {
        ATOM_ENGINE_ASSERT(!aliasedSurface.IsAlias());

        // Create texture alias for the specified subresource
        m_Texture = CreateRef<Texture>(*aliasedSurface.GetTexture(), mipIndex, sliceIndex);

        // Create views
        TextureViewDescription viewDesc;
        viewDesc.FirstMip = mipIndex == TextureView::AllMips ? 0 : mipIndex;
        viewDesc.MipLevels = m_Texture->GetMipLevels();
        viewDesc.FirstSlice = sliceIndex == TextureView::AllSlices ? 0 : sliceIndex;
        viewDesc.ArraySize = m_Texture->GetType() == TextureType::Texture3D ? m_Texture->GetDepth() : m_Texture->GetArraySize();

        if (IsSet(m_Texture->GetFlags() & TextureFlags::RenderTarget))
            m_RTV = CreateScope<TextureViewRT>(this, viewDesc, !IsSet(m_Texture->GetFlags() & TextureFlags::SwapChainBuffer));
        if (IsSet(m_Texture->GetFlags() & TextureFlags::DepthStencil))
            m_DSV = CreateScope<TextureViewDS>(this, viewDesc, !IsSet(m_Texture->GetFlags() & TextureFlags::SwapChainBuffer));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurface::RenderSurface(ID3D12Resource* textureHandle, bool swapChainBuffer, const char* debugName)
    {
        m_Texture = CreateRef<Texture>(textureHandle, swapChainBuffer ? TextureFlags::SwapChainBuffer : TextureFlags::None, debugName);

        // Create views
        TextureViewDescription viewDesc;
        viewDesc.FirstMip = 0;
        viewDesc.MipLevels = m_Texture->GetMipLevels();
        viewDesc.FirstSlice = 0;
        viewDesc.ArraySize = m_Texture->GetType() == TextureType::Texture3D ? m_Texture->GetDepth() : m_Texture->GetArraySize();

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
    ResourceState RenderSurface::GetInitialState() const
    {
        return m_Texture->GetInitialState();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture> RenderSurface::GetTexture() const
    {
        return m_Texture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool RenderSurface::IsSwapChainBuffer() const
    {
        return IsSet(m_Texture->GetFlags() & TextureFlags::SwapChainBuffer);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool RenderSurface::IsAlias() const
    {
        return m_Texture->IsAlias();
    }
}
