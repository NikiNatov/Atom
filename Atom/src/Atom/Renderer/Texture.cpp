#include "atompch.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"

#include "Texture.h"
#include "Device.h"
#include "ResourceStateTracker.h"
#include "Renderer.h"

namespace Atom
{
    // ------------------------------------------------------ Texture --------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    Texture::Texture(TextureType type, const TextureDescription& description, const char* debugName)
        : m_Type(type), m_Description(description)
    {
        ATOM_ENGINE_ASSERT(m_Description.Width && m_Description.Height, "Texture dimensions cannot be 0!");
        ATOM_ENGINE_ASSERT(m_Description.MipLevels, "Texture mip levels cannot be 0!");

        ATOM_ENGINE_ASSERT(!(IsSet(m_Description.UsageFlags & TextureBindFlags::DepthStencil) &&
            (IsSet(m_Description.UsageFlags & TextureBindFlags::RenderTarget) || IsSet(m_Description.UsageFlags & TextureBindFlags::UnorderedAccess))),
            "Texture that has DepthBuffer flag cannot have RenderTarget or UnorderedAccess flag at the same time!");

        ATOM_ENGINE_ASSERT(!(IsSet(m_Description.UsageFlags & TextureBindFlags::DepthStencil) &&
            ((m_Description.Format != TextureFormat::Depth24Stencil8) && (m_Description.Format != TextureFormat::Depth32))),
            "Texture with DepthBuffer usage flag must have a depth format!");

        ATOM_ENGINE_ASSERT(!(IsSet(m_Description.UsageFlags & TextureBindFlags::RenderTarget) &&
            ((m_Description.Format == TextureFormat::Depth24Stencil8) || (m_Description.Format == TextureFormat::Depth32))),
            "Texture with RenderTarget usage flag cannot have a depth format!");

        ATOM_ENGINE_ASSERT(!(IsSet(m_Description.UsageFlags & TextureBindFlags::UnorderedAccess) &&
            ((m_Description.Format == TextureFormat::Depth24Stencil8) || (m_Description.Format == TextureFormat::Depth32))),
            "Texture with UnorderedAccess usage flag cannot have a depth format!");

        auto d3dDevice = Renderer::GetDevice()->GetD3DDevice();

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
        resourceDesc.Flags |= IsSet(m_Description.UsageFlags & TextureBindFlags::RenderTarget) ? D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET : D3D12_RESOURCE_FLAG_NONE;
        resourceDesc.Flags |= IsSet(m_Description.UsageFlags & TextureBindFlags::UnorderedAccess) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
        resourceDesc.Flags |= IsSet(m_Description.UsageFlags & TextureBindFlags::DepthStencil) ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_NONE;

        DXCall(d3dDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_D3DResource)));

#if defined (ATOM_DEBUG)
        String name = debugName;
        DXCall(m_D3DResource->SetName(STRING_TO_WSTRING(name).c_str()));
#endif

        ResourceStateTracker::AddGlobalResourceState(m_D3DResource.Get(), D3D12_RESOURCE_STATE_COMMON);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture::Texture(TextureType type, ID3D12Resource* textureHandle, const char* debugName)
        : m_Type(type)
    {
        m_D3DResource.Attach(textureHandle);

        // Set the description based on the d3d resource
        D3D12_RESOURCE_DESC desc = m_D3DResource->GetDesc();
        m_Description.Width = desc.Width;
        m_Description.Height = desc.Height;
        m_Description.Format = Utils::DXGITextureFormatToAtomFormat(desc.Format);
        m_Description.MipLevels = desc.MipLevels;
        m_Description.UsageFlags |= desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ? TextureBindFlags::RenderTarget : TextureBindFlags::None;
        m_Description.UsageFlags |= desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL ? TextureBindFlags::DepthStencil : TextureBindFlags::None;
        m_Description.UsageFlags |= desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS ? TextureBindFlags::UnorderedAccess : TextureBindFlags::None;
        m_Description.Filter = TextureFilter::Linear;
        m_Description.Wrap = TextureWrap::Repeat;

#if defined (ATOM_DEBUG)
        String name = debugName;
        DXCall(m_D3DResource->SetName(STRING_TO_WSTRING(name).c_str()));
#endif

        ResourceStateTracker::AddGlobalResourceState(m_D3DResource.Get(), D3D12_RESOURCE_STATE_COMMON);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture::~Texture()
    {
        Renderer::GetDevice()->ReleaseResource(m_D3DResource.Detach(), m_Type != TextureType::SwapChainBuffer);
    }

    
    // -----------------------------------------------------------------------------------------------------------------------------
    TextureType Texture::GetType() const
    {
        return m_Type;
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
    u32 Texture::GetMipLevels() const
    {
        return m_Description.MipLevels;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureBindFlags Texture::GetBindFlags() const
    {
        return m_Description.UsageFlags;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureFilter Texture::GetFilter() const
    {
        return m_Description.Filter;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureWrap Texture::GetWrap() const
    {
        return m_Description.Wrap;
    }

    // ------------------------------------------------- Texture2D -----------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    Texture2D::Texture2D(const TextureDescription& description, const char* debugName)
        : Texture(TextureType::Texture2D, description, debugName)
    {
        CreateViews();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture2D::Texture2D(ID3D12Resource* textureHandle, const char* debugName)
        : Texture(TextureType::Texture2D, textureHandle, debugName)
    {
        CreateViews();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture2D::~Texture2D()
    {
        Renderer::GetDevice()->GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_SRVDescriptor, true);

        if (IsSet(m_Description.UsageFlags & TextureBindFlags::UnorderedAccess))
        {
            for (u32 i = 0; i < m_Description.MipLevels; i++)
            {
                Renderer::GetDevice()->GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_UAVDescriptors[i], true);
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    D3D12_CPU_DESCRIPTOR_HANDLE Texture2D::GetSRV() const
    {
        return m_SRVDescriptor;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    D3D12_CPU_DESCRIPTOR_HANDLE Texture2D::GetUAV(u32 mip) const
    {
        ATOM_ENGINE_ASSERT(mip < m_Description.MipLevels); 
        return m_UAVDescriptors[mip];
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Texture2D::CreateViews()
    {
        auto dx12Device = Renderer::GetDevice();

        // Create SRV
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = Utils::AtomTextureFormatToSRVFormat(m_Description.Format);
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = m_Description.MipLevels;

        m_SRVDescriptor = dx12Device->GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocateDescriptor();
        dx12Device->GetD3DDevice()->CreateShaderResourceView(m_D3DResource.Get(), &srvDesc, m_SRVDescriptor);

        // Create UAVs
        if (IsSet(m_Description.UsageFlags & TextureBindFlags::UnorderedAccess))
        {
            m_UAVDescriptors.resize(m_Description.MipLevels);
            for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
            {
                D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
                uavDesc.Format = Utils::AtomTextureFormatToUAVFormat(m_Description.Format);
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                uavDesc.Texture2D.MipSlice = mip;

                m_UAVDescriptors[mip] = dx12Device->GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocateDescriptor();
                dx12Device->GetD3DDevice()->CreateUnorderedAccessView(m_D3DResource.Get(), nullptr, &uavDesc, m_UAVDescriptors[mip]);
            }
        }
    }

    // ------------------------------------------------ RenderTexture2D ------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    RenderTexture2D::RenderTexture2D(const TextureDescription& description, bool swapChainBuffer, const char* debugName)
        : Texture(swapChainBuffer ? TextureType::SwapChainBuffer : TextureType::Texture2D, description, debugName)
    {
        m_Description.UsageFlags |= TextureBindFlags::RenderTarget;
        CreateViews();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderTexture2D::RenderTexture2D(ID3D12Resource* textureHandle, bool swapChainBuffer, const char* debugName)
        : Texture(swapChainBuffer ? TextureType::SwapChainBuffer : TextureType::Texture2D, textureHandle, debugName)
    {
        m_Description.UsageFlags |= TextureBindFlags::RenderTarget;
        CreateViews();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderTexture2D::~RenderTexture2D()
    {
        Renderer::GetDevice()->GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_SRVDescriptor, m_Type != TextureType::SwapChainBuffer);

        for (u32 i = 0; i < m_Description.MipLevels; i++)
        {
            Renderer::GetDevice()->GetCPUDescriptorHeap(DescriptorHeapType::RenderTarget)->ReleaseDescriptor(m_RTVDescriptors[i], m_Type != TextureType::SwapChainBuffer);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    D3D12_CPU_DESCRIPTOR_HANDLE RenderTexture2D::GetSRV() const
    {
        return m_SRVDescriptor;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    D3D12_CPU_DESCRIPTOR_HANDLE RenderTexture2D::GetRTV(u32 mip) const
    {
        ATOM_ENGINE_ASSERT(mip < m_Description.MipLevels); 
        return m_RTVDescriptors[mip];
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderTexture2D::CreateViews()
    {
        auto dx12Device = Renderer::GetDevice();

        // Create SRV
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = Utils::AtomTextureFormatToSRVFormat(m_Description.Format);
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = m_Description.MipLevels;

        m_SRVDescriptor = dx12Device->GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocateDescriptor();
        dx12Device->GetD3DDevice()->CreateShaderResourceView(m_D3DResource.Get(), &srvDesc, m_SRVDescriptor);

        // Create RTVs
        m_RTVDescriptors.resize(m_Description.MipLevels);
        for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
        {
            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = Utils::AtomTextureFormatToRTVFormat(m_Description.Format);
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice = mip;

            m_RTVDescriptors[mip] = dx12Device->GetCPUDescriptorHeap(DescriptorHeapType::RenderTarget)->AllocateDescriptor();
            dx12Device->GetD3DDevice()->CreateRenderTargetView(m_D3DResource.Get(), &rtvDesc, m_RTVDescriptors[mip]);
        }
    }

    // --------------------------------------------------- DepthBuffer -------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    DepthBuffer::DepthBuffer(const TextureDescription& description, const char* debugName)
        : Texture(TextureType::Texture2D, description, debugName)
    {
        ATOM_ENGINE_ASSERT(m_Description.Format == TextureFormat::Depth24Stencil8 || m_Description.Format == TextureFormat::Depth32);
        m_Description.UsageFlags |= TextureBindFlags::DepthStencil;

        CreateViews();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DepthBuffer::DepthBuffer(ID3D12Resource* textureHandle, const char* debugName)
        : Texture(TextureType::Texture2D, textureHandle, debugName)
    {
        ATOM_ENGINE_ASSERT(m_Description.Format == TextureFormat::Depth24Stencil8 || m_Description.Format == TextureFormat::Depth32);
        m_Description.UsageFlags |= TextureBindFlags::DepthStencil;

        CreateViews();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DepthBuffer::~DepthBuffer()
    {
        Renderer::GetDevice()->GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_SRVDescriptor, true);

        for (u32 i = 0; i < m_Description.MipLevels; i++)
        {
            Renderer::GetDevice()->GetCPUDescriptorHeap(DescriptorHeapType::DepthStencil)->ReleaseDescriptor(m_DSVDescriptors[i], true);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    D3D12_CPU_DESCRIPTOR_HANDLE DepthBuffer::GetSRV() const
    {
        return m_SRVDescriptor;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    D3D12_CPU_DESCRIPTOR_HANDLE DepthBuffer::GetDSV(u32 mip) const
    {
        ATOM_ENGINE_ASSERT(mip < m_Description.MipLevels); 
        return m_DSVDescriptors[mip];
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DepthBuffer::CreateViews()
    {
        auto dx12Device = Renderer::GetDevice();

        // Create SRV
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = Utils::AtomTextureFormatToSRVFormat(m_Description.Format);
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = m_Description.MipLevels;

        m_SRVDescriptor = dx12Device->GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocateDescriptor();
        dx12Device->GetD3DDevice()->CreateShaderResourceView(m_D3DResource.Get(), &srvDesc, m_SRVDescriptor);

        // Create DSVs
        m_DSVDescriptors.resize(m_Description.MipLevels);
        for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
        {
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = Utils::AtomTextureFormatToDSVFormat(m_Description.Format);
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = mip;

            m_DSVDescriptors[mip] = dx12Device->GetCPUDescriptorHeap(DescriptorHeapType::DepthStencil)->AllocateDescriptor();
            dx12Device->GetD3DDevice()->CreateDepthStencilView(m_D3DResource.Get(), &dsvDesc, m_DSVDescriptors[mip]);
        }
    }
}
