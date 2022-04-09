#include "atompch.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DX12Texture.h"
#include "DX12Device.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    DX12Texture::DX12Texture(TextureType type, const TextureDescription& description)
        : m_Type(type), m_Description(description)
    {
        ATOM_ENGINE_ASSERT(m_Description.Width && m_Description.Height, "Texture dimensions cannot be 0!");
        ATOM_ENGINE_ASSERT(m_Description.MipLevels, "Texture mip levels cannot be 0!");
        ATOM_ENGINE_ASSERT(m_Description.UsageFlags, "Usage flags not set!");

        ATOM_ENGINE_ASSERT(!((m_Description.UsageFlags & TextureUsage::DepthBuffer) &&
                            ((m_Description.UsageFlags & TextureUsage::RenderTarget) || (m_Description.UsageFlags & TextureUsage::UnorderedAccess))),
                            "Texture that has DepthBuffer flag cannot have RenderTarget or UnorderedAccess flag at the same time!");

        ATOM_ENGINE_ASSERT(!((m_Description.UsageFlags & TextureUsage::DepthBuffer) &&
                            ((m_Description.Format != TextureFormat::Depth24Stencil8) && (m_Description.Format != TextureFormat::Depth32))), 
                            "Texture with DepthBuffer usage flag must have a depth format!");

        ATOM_ENGINE_ASSERT(!((m_Description.UsageFlags & TextureUsage::RenderTarget) &&
                            ((m_Description.Format == TextureFormat::Depth24Stencil8) || (m_Description.Format == TextureFormat::Depth32))),
                            "Texture with RenderTarget usage flag cannot have a depth format!");

        ATOM_ENGINE_ASSERT(!((m_Description.UsageFlags & TextureUsage::UnorderedAccess) &&
                            ((m_Description.Format == TextureFormat::Depth24Stencil8) || (m_Description.Format == TextureFormat::Depth32))),
                            "Texture with UnorderedAccess usage flag cannot have a depth format!");

        auto d3dDevice = Renderer::GetDevice().As<DX12Device>()->GetD3DDevice();

        // Create the resource
        D3D12_HEAP_PROPERTIES heapProperties = {};
        heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resourceDesc.Width = m_Description.Width;
        resourceDesc.Height = m_Description.Height;
        resourceDesc.DepthOrArraySize = m_Type == TextureType::TextureCube ? 6 : 1;
        resourceDesc.MipLevels = m_Description.MipLevels;
        resourceDesc.Format = Utils::AtomTextureFormatToD3D12(m_Description.Format);
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resourceDesc.Flags |= (m_Description.UsageFlags & TextureUsage::RenderTarget) ? D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET : D3D12_RESOURCE_FLAG_NONE;
        resourceDesc.Flags |= (m_Description.UsageFlags & TextureUsage::UnorderedAccess) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
        resourceDesc.Flags |= (m_Description.UsageFlags & TextureUsage::DepthBuffer) ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_NONE;

        DXCall(d3dDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_D3DResource)));
        DXCall(m_D3DResource->SetName(WString(m_Description.DebugName.begin(), m_Description.DebugName.end()).c_str()));

    }
    
    // -----------------------------------------------------------------------------------------------------------------------------
    DX12Texture::DX12Texture(TextureType type, u64 textureHandle)
        : m_Type(type)
    {
        m_D3DResource.Attach((ID3D12Resource*)textureHandle);

        // Set the description based on the d3d resource
        D3D12_RESOURCE_DESC desc = m_D3DResource->GetDesc();
        char name[100]{ 0 };
        u32 bufferSize = sizeof(name) - 1;
        m_D3DResource->GetPrivateData(WKPDID_D3DDebugObjectNameW, &bufferSize, name);

        m_Description.DebugName = name;
        m_Description.Width = desc.Width;
        m_Description.Height = desc.Height;
        m_Description.Format = Utils::DXGITextureFormatToAtomFormat(desc.Format);
        m_Description.MipLevels = desc.MipLevels;
        m_Description.UsageFlags |= desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ? TextureUsage::RenderTarget : 0;
        m_Description.UsageFlags |= desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL ? TextureUsage::DepthBuffer : 0;
        m_Description.UsageFlags |= desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS ? TextureUsage::UnorderedAccess : 0;
        m_Description.Filter = TextureFilter::Linear;
        m_Description.Wrap = TextureWrap::Repeat;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12Texture::~DX12Texture()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12Texture::Release()
    {
        m_D3DResource.Reset();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12Texture::DeferredRelease()
    {
        Renderer::GetDevice().As<DX12Device>()->ReleaseResource(m_D3DResource.Detach());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const String& DX12Texture::GetDebugName() const
    {
        return m_Description.DebugName;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureType DX12Texture::GetType() const
    {
        return m_Type;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureFormat DX12Texture::GetFormat() const
    {
        return m_Description.Format;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12Texture::GetWidth() const
    {
        return m_Description.Width;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12Texture::GetHeight() const
    {
        return m_Description.Height;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12Texture::GetMipLevels() const
    {
        return m_Description.MipLevels;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u8 DX12Texture::GetUsageFlags() const
    {
        return m_Description.UsageFlags;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureFilter DX12Texture::GetFilter() const
    {
        return m_Description.Filter;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureWrap DX12Texture::GetWrap() const
    {
        return m_Description.Wrap;
    }
}

#endif // ATOM_PLATFORM_WINDOWS