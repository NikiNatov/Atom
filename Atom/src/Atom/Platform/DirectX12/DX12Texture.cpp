#include "atompch.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DX12Texture.h"
#include "DX12Device.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    DX12Texture2D::DX12Texture2D(const TextureDescription& description)
        : m_Description(description), m_Device(*(Renderer::GetDevice().As<DX12Device>()))
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

        auto d3dDevice = m_Device.GetD3DDevice();

        // Create the resource
        D3D12_HEAP_PROPERTIES heapProperties = {};
        heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resourceDesc.Alignment;
        resourceDesc.Width = m_Description.Width;
        resourceDesc.Height = m_Description.Height;
        resourceDesc.DepthOrArraySize = 1;
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

        CreateViewsAndSampler();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12Texture2D::DX12Texture2D(const String& filename, const TextureDescription& description)
        : m_Description(description), m_Device(*(Renderer::GetDevice().As<DX12Device>()))
    {
        // TODO: Implement loading from file
#if 0
        ATOM_ENGINE_ASSERT(m_Description.MipLevels, "Texture mip levels cannot be 0!");

        // Since the texture is loaded from a file it is assumed that it will be used as a shader resource only
        m_Description.UsageFlags = TextureUsage::ShaderResource;

        CreateViewsAndSampler();
#endif
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12Texture2D::DX12Texture2D(DX12Device& device, wrl::ComPtr<ID3D12Resource2> resource, TextureFilter filter, TextureWrap wrap)
        : m_D3DResource(resource), m_Device(device)
    {
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
        m_Description.Filter = filter;
        m_Description.Wrap = wrap;

        CreateViewsAndSampler();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12Texture2D::~DX12Texture2D()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12Texture2D::Release()
    {
        m_D3DResource.Reset();

        m_ShaderResourceView.Release();
        m_Sampler.Release();

        for (auto& descriptor : m_RenderTargetViews)
        {
            descriptor.Release();
        }

        for (auto& descriptor : m_UnorderedAccessViews)
        {
            descriptor.Release();
        }

        for (auto& descriptor : m_DepthStencilViews)
        {
            descriptor.Release();
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12Texture2D::DeferredRelease()
    {
        m_Device.ReleaseResource(m_D3DResource.Detach());

        m_ShaderResourceView.DeferredRelease();
        m_Sampler.DeferredRelease();

        for (auto& descriptor : m_RenderTargetViews)
        {
            descriptor.DeferredRelease();
        }

        for (auto& descriptor : m_UnorderedAccessViews)
        {
            descriptor.DeferredRelease();
        }

        for (auto& descriptor : m_DepthStencilViews)
        {
            descriptor.DeferredRelease();
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12Texture2D::SetData(const byte* data)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    byte* DX12Texture2D::GetData() const
    {
        return nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const String& DX12Texture2D::GetDebugName() const
    {
        return m_Description.DebugName;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureFormat DX12Texture2D::GetFormat() const
    {
        return m_Description.Format;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12Texture2D::GetWidth() const
    {
        return m_Description.Width;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12Texture2D::GetHeight() const
    {
        return m_Description.Height;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12Texture2D::GetMipLevels() const
    {
        return m_Description.MipLevels;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u8 DX12Texture2D::GetUsageFlags() const
    {
        return m_Description.UsageFlags;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureFilter DX12Texture2D::GetFilter() const
    {
        return m_Description.Filter;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureWrap DX12Texture2D::GetWrap() const
    {
        return m_Description.Wrap;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<DX12Texture2D> DX12Texture2D::CreateFromD3DResource(DX12Device& device, wrl::ComPtr<ID3D12Resource2> resource, TextureFilter filter, TextureWrap wrap)
    {
        return CreateRef<DX12Texture2D>(device, resource, filter, wrap);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12Texture2D::CreateViewsAndSampler()
    {
        // Create shader resource view
        if (m_Description.UsageFlags & TextureUsage::ShaderResource)
        {
            m_ShaderResourceView = m_Device.AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            // Create RTV
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = Utils::AtomTextureFormatToSRVFormat(m_Description.Format);
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Texture2D.MipLevels = m_Description.MipLevels;
            srvDesc.Texture2D.MostDetailedMip = 0;

            m_Device.GetD3DDevice()->CreateShaderResourceView(m_D3DResource.Get(), &srvDesc, m_ShaderResourceView.GetCPUHandle());
        }

        // Create render target views
        if (m_Description.UsageFlags & TextureUsage::RenderTarget)
        {
            m_RenderTargetViews.resize(m_Description.MipLevels);
            for (u32 i = 0; i < m_Description.MipLevels; i++)
            {
                // Allocate descriptor
                m_RenderTargetViews[i] = m_Device.AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

                // Create RTV
                D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
                rtvDesc.Format = Utils::AtomTextureFormatToD3D12(m_Description.Format);
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice = i;

                m_Device.GetD3DDevice()->CreateRenderTargetView(m_D3DResource.Get(), &rtvDesc, m_RenderTargetViews[i].GetCPUHandle());
            }
        }

        // Create unordered access views
        if (m_Description.UsageFlags & TextureUsage::UnorderedAccess)
        {
            m_UnorderedAccessViews.resize(m_Description.MipLevels);
            for (u32 i = 0; i < m_Description.MipLevels; i++)
            {
                // Allocate descriptor
                m_UnorderedAccessViews[i] = m_Device.AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

                // Create RTV
                D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
                uavDesc.Format = Utils::AtomTextureFormatToD3D12(m_Description.Format);
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                uavDesc.Texture2D.MipSlice = i;

                m_Device.GetD3DDevice()->CreateUnorderedAccessView(m_D3DResource.Get(), nullptr, &uavDesc, m_UnorderedAccessViews[i].GetCPUHandle());
            }
        }

        // Create depth-stencil views
        if (m_Description.UsageFlags & TextureUsage::DepthBuffer)
        {
            m_DepthStencilViews.resize(m_Description.MipLevels);
            for (u32 i = 0; i < m_Description.MipLevels; i++)
            {
                // Allocate descriptor
                m_DepthStencilViews[i] = m_Device.AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

                // Create RTV
                D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
                dsvDesc.Format = Utils::AtomTextureFormatToDSVFormat(m_Description.Format);
                dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                dsvDesc.Texture2D.MipSlice = i;

                m_Device.GetD3DDevice()->CreateDepthStencilView(m_D3DResource.Get(), &dsvDesc, m_DepthStencilViews[i].GetCPUHandle());
            }
        }

        // Create sampler
        m_Sampler = m_Device.AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

        D3D12_SAMPLER_DESC samplerDesc = {};
        samplerDesc.AddressU = Utils::AtomTextureWrapToD3D12(m_Description.Wrap);
        samplerDesc.AddressV = Utils::AtomTextureWrapToD3D12(m_Description.Wrap);
        samplerDesc.AddressW = Utils::AtomTextureWrapToD3D12(m_Description.Wrap);
        samplerDesc.Filter = Utils::AtomTextureFilterToD3D12(m_Description.Filter);
        samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        samplerDesc.MinLOD = 0.0;
        samplerDesc.MaxLOD = m_Description.MipLevels - 1;
        samplerDesc.MaxAnisotropy = m_Description.Filter == TextureFilter::Anisotropic ? D3D12_REQ_MAXANISOTROPY : 1;

        m_Device.GetD3DDevice()->CreateSampler(&samplerDesc, m_Sampler.GetCPUHandle());
    }

}

#endif // ATOM_PLATFORM_WINDOWS