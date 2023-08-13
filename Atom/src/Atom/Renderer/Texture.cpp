#include "atompch.h"
#include "Texture.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Core/Application.h"

#include "Atom/Renderer/Device.h"
#include "Atom/Renderer/ResourceStateTracker.h"
#include "Atom/Renderer/TextureView.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Texture::Texture(const TextureDescription& description, const char* debugName)
        : m_Description(description)
    {
        ATOM_ENGINE_ASSERT(m_Description.Width && m_Description.Height && m_Description.Depth && m_Description.ArraySize, "Texture dimensions cannot be 0!");
        ATOM_ENGINE_ASSERT(m_Description.MipLevels, "Texture mip levels cannot be 0!");

        bool isShaderResource = IsSet(m_Description.Flags & TextureFlags::ShaderResource);
        bool isRenderTarget = IsSet(m_Description.Flags & TextureFlags::RenderTarget);
        bool isDepthStencil = IsSet(m_Description.Flags & TextureFlags::DepthStencil);
        bool isUnorderedAccess = IsSet(m_Description.Flags & TextureFlags::UnorderedAccess);
        bool isCubeMap = IsSet(m_Description.Flags & TextureFlags::CubeMap);

        ATOM_ENGINE_ASSERT(!isCubeMap || m_Description.Type == TextureType::Texture2D, "Cube maps must be of type Texture2D!");
        ATOM_ENGINE_ASSERT(!isCubeMap || m_Description.ArraySize == 6, "Cube maps must have array size of 6!");
        ATOM_ENGINE_ASSERT(!isDepthStencil || m_Description.Type != TextureType::Texture3D, "3D textures cannot be used as depth stencil buffers!");

        ATOM_ENGINE_ASSERT(!(isDepthStencil && (isRenderTarget || isUnorderedAccess)),
            "Texture that has DepthBuffer flag cannot have RenderTarget or UnorderedAccess flag at the same time!");

        ATOM_ENGINE_ASSERT(!(isDepthStencil && ((m_Description.Format != TextureFormat::Depth24Stencil8) && (m_Description.Format != TextureFormat::Depth32))),
            "Texture with DepthBuffer usage flag must have a depth format!");

        ATOM_ENGINE_ASSERT(!(isRenderTarget && ((m_Description.Format == TextureFormat::Depth24Stencil8) || (m_Description.Format == TextureFormat::Depth32))),
            "Texture with RenderTarget usage flag cannot have a depth format!");

        ATOM_ENGINE_ASSERT(!(isUnorderedAccess && ((m_Description.Format == TextureFormat::Depth24Stencil8) || (m_Description.Format == TextureFormat::Depth32))),
            "Texture with UnorderedAccess usage flag cannot have a depth format!");

        auto d3dDevice = Device::Get().GetD3DDevice();

        // Create the resource
        D3D12_HEAP_PROPERTIES heapProperties = {};
        heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = Utils::AtomTextureTypeToD3D12(m_Description.Type);
        resourceDesc.Format = Utils::AtomTextureFormatToD3D12(m_Description.Format);
        resourceDesc.Width = m_Description.Width;
        resourceDesc.Height = m_Description.Height;
        resourceDesc.DepthOrArraySize = m_Description.Type == TextureType::Texture3D ? m_Description.Depth : m_Description.ArraySize;
        resourceDesc.MipLevels = m_Description.MipLevels;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resourceDesc.Flags |= !isShaderResource ? D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE : D3D12_RESOURCE_FLAG_NONE;
        resourceDesc.Flags |= isRenderTarget ? D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET : D3D12_RESOURCE_FLAG_NONE;
        resourceDesc.Flags |= isDepthStencil ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_NONE;
        resourceDesc.Flags |= isUnorderedAccess ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

        D3D12_CLEAR_VALUE clearValue = {};
        if (isDepthStencil)
        {
            clearValue.Format = Utils::AtomTextureFormatToDSVFormat(m_Description.Format);
            clearValue.DepthStencil.Depth = m_Description.ClearValue.DepthStencil.DepthValue;
            clearValue.DepthStencil.Stencil = m_Description.ClearValue.DepthStencil.StencilValue;
        }
        else if(isRenderTarget)
        {
            clearValue.Format = Utils::AtomTextureFormatToRTVFormat(m_Description.Format);
            clearValue.Color[0] = m_Description.ClearValue.Color.r;
            clearValue.Color[1] = m_Description.ClearValue.Color.g;
            clearValue.Color[2] = m_Description.ClearValue.Color.b;
            clearValue.Color[3] = m_Description.ClearValue.Color.a;
        }

        D3D12_RESOURCE_STATES initialState = Utils::AtomResourceStateToD3D12(m_Description.InitialState);

        DXCall(d3dDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, initialState,
            isRenderTarget || isDepthStencil ? &clearValue : nullptr, IID_PPV_ARGS(&m_D3DResource)));

#if defined (ATOM_DEBUG)
        String name = debugName;
        DXCall(m_D3DResource->SetName(STRING_TO_WSTRING(name).c_str()));
#endif

        ResourceStateTracker::AddGlobalResourceState(m_D3DResource.Get(), initialState);

        // Create views
        TextureViewDescription viewDesc;
        viewDesc.FirstMip = 0;
        viewDesc.MipLevels = m_Description.MipLevels;
        viewDesc.FirstSlice = 0;
        viewDesc.ArraySize = m_Description.Type == TextureType::Texture3D ? m_Description.Depth : m_Description.ArraySize;

        if (IsSet(m_Description.Flags & TextureFlags::ShaderResource))
            m_SRV = CreateScope<TextureViewRO>(this, viewDesc, !IsSet(m_Description.Flags & TextureFlags::SwapChainBuffer));
        if (IsSet(m_Description.Flags & TextureFlags::UnorderedAccess))
            m_UAV = CreateScope<TextureViewRW>(this, viewDesc, !IsSet(m_Description.Flags & TextureFlags::SwapChainBuffer));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture::Texture(const Texture& aliasedTexture, u32 mipIndex, u32 sliceIndex)
        : m_Description(aliasedTexture.m_Description), m_D3DResource(aliasedTexture.m_D3DResource), m_IsAlias(true)
    {
        ATOM_ENGINE_ASSERT(!aliasedTexture.IsAlias());

        m_Description.Width = mipIndex == TextureView::AllMips ? aliasedTexture.m_Description.Width : aliasedTexture.m_Description.Width >> mipIndex;
        m_Description.Height = mipIndex == TextureView::AllMips ? aliasedTexture.m_Description.Height : aliasedTexture.m_Description.Height >> mipIndex;

        if(m_Description.Type == TextureType::Texture3D)
            m_Description.Depth = mipIndex == TextureView::AllMips ? aliasedTexture.m_Description.Depth : aliasedTexture.m_Description.Depth >> mipIndex;
        else
            m_Description.ArraySize = sliceIndex == TextureView::AllSlices ? aliasedTexture.m_Description.ArraySize : 1;

        m_Description.MipLevels = mipIndex == TextureView::AllMips ? aliasedTexture.m_Description.MipLevels : 1;

        // Create views
        TextureViewDescription viewDesc;
        viewDesc.FirstMip = mipIndex == TextureView::AllMips ? 0 : mipIndex;
        viewDesc.MipLevels = m_Description.MipLevels;
        viewDesc.FirstSlice = sliceIndex == TextureView::AllSlices ? 0 : sliceIndex;
        viewDesc.ArraySize = m_Description.Type == TextureType::Texture3D ? m_Description.Depth : m_Description.ArraySize;

        if (IsSet(m_Description.Flags & TextureFlags::ShaderResource))
            m_SRV = CreateScope<TextureViewRO>(this, viewDesc, !IsSet(m_Description.Flags & TextureFlags::SwapChainBuffer));
        if (IsSet(m_Description.Flags & TextureFlags::UnorderedAccess))
            m_UAV = CreateScope<TextureViewRW>(this, viewDesc, !IsSet(m_Description.Flags & TextureFlags::SwapChainBuffer));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture::Texture(ID3D12Resource* textureHandle, TextureFlags additionalFlags, const char* debugName)
    {
        m_D3DResource.Attach(textureHandle);

        // Set the description based on the d3d resource
        D3D12_RESOURCE_DESC desc = m_D3DResource->GetDesc();
        m_Description.Type = Utils::D3D12TextureTypeAtom(desc.Dimension);
        m_Description.Format = Utils::DXGITextureFormatToAtomFormat(desc.Format);
        m_Description.Width = desc.Width;
        m_Description.Height = desc.Height;

        if(m_Description.Type == TextureType::Texture3D)
            m_Description.Depth = desc.DepthOrArraySize;
        else
            m_Description.ArraySize = desc.DepthOrArraySize;

        m_Description.MipLevels = desc.MipLevels;
        m_Description.Flags |= desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ? TextureFlags::RenderTarget : TextureFlags::None;
        m_Description.Flags |= desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL ? TextureFlags::DepthStencil : TextureFlags::None;
        m_Description.Flags |= desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS ? TextureFlags::UnorderedAccess : TextureFlags::None;
        m_Description.Flags |= additionalFlags;

        bool isShaderResource = IsSet(m_Description.Flags & TextureFlags::ShaderResource);
        bool isRenderTarget = IsSet(m_Description.Flags & TextureFlags::RenderTarget);
        bool isDepthStencil = IsSet(m_Description.Flags & TextureFlags::DepthStencil);
        bool isUnorderedAccess = IsSet(m_Description.Flags & TextureFlags::UnorderedAccess);
        bool isCubeMap = IsSet(m_Description.Flags & TextureFlags::CubeMap);

        ATOM_ENGINE_ASSERT(!isCubeMap || m_Description.Type == TextureType::Texture2D, "Cube maps must be of type Texture2D!");
        ATOM_ENGINE_ASSERT(!isCubeMap || m_Description.ArraySize == 6, "Cube maps must have array size of 6!");
        ATOM_ENGINE_ASSERT(!isDepthStencil || m_Description.Type != TextureType::Texture3D, "3D textures cannot be used as depth stencil buffers!");

        ATOM_ENGINE_ASSERT(!(isDepthStencil && (isRenderTarget || isUnorderedAccess)),
            "Texture that has DepthBuffer flag cannot have RenderTarget or UnorderedAccess flag at the same time!");

        ATOM_ENGINE_ASSERT(!(isDepthStencil && ((m_Description.Format != TextureFormat::Depth24Stencil8) && (m_Description.Format != TextureFormat::Depth32))),
            "Texture with DepthBuffer usage flag must have a depth format!");

        ATOM_ENGINE_ASSERT(!(isRenderTarget && ((m_Description.Format == TextureFormat::Depth24Stencil8) || (m_Description.Format == TextureFormat::Depth32))),
            "Texture with RenderTarget usage flag cannot have a depth format!");

        ATOM_ENGINE_ASSERT(!(isUnorderedAccess && ((m_Description.Format == TextureFormat::Depth24Stencil8) || (m_Description.Format == TextureFormat::Depth32))),
            "Texture with UnorderedAccess usage flag cannot have a depth format!");

#if defined (ATOM_DEBUG)
        String name = debugName;
        DXCall(m_D3DResource->SetName(STRING_TO_WSTRING(name).c_str()));
#endif

        ResourceStateTracker::AddGlobalResourceState(m_D3DResource.Get(), D3D12_RESOURCE_STATE_COMMON);

        // Create views
        TextureViewDescription viewDesc;
        viewDesc.FirstMip = 0;
        viewDesc.MipLevels = m_Description.MipLevels;
        viewDesc.FirstSlice = 0;
        viewDesc.ArraySize = m_Description.Type == TextureType::Texture3D ? m_Description.Depth : m_Description.ArraySize;

        if (IsSet(m_Description.Flags & TextureFlags::ShaderResource))
            m_SRV = CreateScope<TextureViewRO>(this, viewDesc, !IsSet(m_Description.Flags & TextureFlags::SwapChainBuffer));
        if (IsSet(m_Description.Flags & TextureFlags::UnorderedAccess))
            m_UAV = CreateScope<TextureViewRW>(this, viewDesc, !IsSet(m_Description.Flags & TextureFlags::SwapChainBuffer));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture::~Texture()
    {
        if(!m_IsAlias)
            Device::Get().ReleaseResource(m_D3DResource.Detach(), !IsSet(m_Description.Flags & TextureFlags::SwapChainBuffer));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture::Texture(Texture&& rhs) noexcept
        : m_D3DResource(std::move(rhs.m_D3DResource)), m_Description(std::move(rhs.m_Description)), m_SRV(std::move(rhs.m_SRV)), m_UAV(std::move(rhs.m_UAV)), m_IsAlias(rhs.m_IsAlias)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture& Texture::operator=(Texture&& rhs) noexcept
    {
        if (this != &rhs)
        {
            Device::Get().ReleaseResource(m_D3DResource.Detach(), !IsSet(m_Description.Flags & TextureFlags::SwapChainBuffer));

            m_D3DResource = std::move(rhs.m_D3DResource);
            m_Description = std::move(rhs.m_Description);
            m_SRV = std::move(rhs.m_SRV);
            m_UAV = std::move(rhs.m_UAV);
            m_IsAlias = rhs.m_IsAlias;
        }

        return *this;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureType Texture::GetType() const
    {
        return m_Description.Type;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureFormat Texture::GetFormat() const
    {
        return m_Description.Format;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Texture::GetWidth() const
    {
        return m_Description.Width;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Texture::GetHeight() const
    {
        return m_Description.Height;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Texture::GetDepth() const
    {
        return m_Description.Depth;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Texture::GetArraySize() const
    {
        return m_Description.ArraySize;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Texture::GetMipLevels() const
    {
        return m_Description.MipLevels;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureFlags Texture::GetFlags() const
    {
        return m_Description.Flags;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const TextureDescription& Texture::GetDescription() const
    {
        return m_Description;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool Texture::IsAlias() const
    {
        return m_IsAlias;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const ClearValue& Texture::GetClearValue() const
    {
        return m_Description.ClearValue;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ResourceState Texture::GetInitialState() const
    {
        return m_Description.InitialState;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Texture::CalculateMaxMipCount(u32 width, u32 height)
    {
        return (u32)glm::log2((f32)glm::max(width, height)) + 1;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Texture::CalculateSubresource(u32 mip, u32 slice, u32 mipCount, u32 arraySize)
    {
        return D3D12CalcSubresource(mip, slice, 0, mipCount, arraySize);
    }
}
