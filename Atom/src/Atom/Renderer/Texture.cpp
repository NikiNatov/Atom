#include "atompch.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Core/Application.h"

#include "Texture.h"
#include "Device.h"
#include "ResourceStateTracker.h"
#include "Renderer.h"

#include "stb_image.h"
#include <glm/glm.hpp>

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

        auto d3dDevice = Device::Get().GetD3DDevice();

        // Create the resource
        D3D12_HEAP_PROPERTIES heapProperties = {};
        heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

        bool isRenderTarget = IsSet(m_Description.UsageFlags & TextureBindFlags::RenderTarget);
        bool isDepthStencil = IsSet(m_Description.UsageFlags & TextureBindFlags::DepthStencil);

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
        resourceDesc.Flags |= isRenderTarget ? D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET : D3D12_RESOURCE_FLAG_NONE;
        resourceDesc.Flags |= isDepthStencil ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_NONE;
        resourceDesc.Flags |= IsSet(m_Description.UsageFlags & TextureBindFlags::UnorderedAccess) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

        D3D12_CLEAR_VALUE clearValue = {};
        if (IsSet(m_Description.UsageFlags & TextureBindFlags::DepthStencil))
        {
            clearValue.Format = Utils::AtomTextureFormatToDSVFormat(m_Description.Format);
            clearValue.DepthStencil.Depth = m_Description.ClearValue.DepthStencil.DepthValue;
            clearValue.DepthStencil.Stencil = m_Description.ClearValue.DepthStencil.StencilValue;
        }
        else if(IsSet(m_Description.UsageFlags & TextureBindFlags::RenderTarget))
        {
            clearValue.Format = Utils::AtomTextureFormatToRTVFormat(m_Description.Format);
            clearValue.Color[0] = m_Description.ClearValue.Color.r;
            clearValue.Color[1] = m_Description.ClearValue.Color.g;
            clearValue.Color[2] = m_Description.ClearValue.Color.b;
            clearValue.Color[3] = m_Description.ClearValue.Color.a;
        }

        DXCall(d3dDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, 
            isRenderTarget || isDepthStencil ? &clearValue : nullptr, IID_PPV_ARGS(&m_D3DResource)));

#if defined (ATOM_DEBUG)
        String name = debugName;
        DXCall(m_D3DResource->SetName(STRING_TO_WSTRING(name).c_str()));
#endif

        ResourceStateTracker::AddGlobalResourceState(m_D3DResource.Get(), D3D12_RESOURCE_STATE_COMMON);

        // Create descriptors
        CreateSRV();
        CreateSampler();
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

        // Create descriptors
        CreateSRV();
        CreateSampler();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture::~Texture()
    {
        Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_SRVDescriptor, m_Type != TextureType::SwapChainBuffer);
        Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::Sampler)->ReleaseDescriptor(m_SamplerDescriptor, m_Type != TextureType::SwapChainBuffer);
        Device::Get().ReleaseResource(m_D3DResource.Detach(), m_Type != TextureType::SwapChainBuffer);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture::Texture(Texture&& rhs) noexcept
        : m_D3DResource(rhs.m_D3DResource), m_SRVDescriptor(rhs.m_SRVDescriptor), m_SamplerDescriptor(rhs.m_SamplerDescriptor), m_Type(rhs.m_Type), m_Description(rhs.m_Description)
    {
        rhs.m_D3DResource = nullptr;
        rhs.m_SRVDescriptor = { 0 };
        rhs.m_SamplerDescriptor = { 0 };
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture& Texture::operator=(Texture&& rhs) noexcept
    {
        if (this != &rhs)
        {
            Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_SRVDescriptor, m_Type != TextureType::SwapChainBuffer);
            Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::Sampler)->ReleaseDescriptor(m_SamplerDescriptor, m_Type != TextureType::SwapChainBuffer);
            Device::Get().ReleaseResource(m_D3DResource.Detach(), m_Type != TextureType::SwapChainBuffer);

            m_D3DResource = rhs.m_D3DResource;
            m_SRVDescriptor = rhs.m_SRVDescriptor;
            m_SamplerDescriptor = rhs.m_SamplerDescriptor;
            m_Type = rhs.m_Type;
            m_Description = rhs.m_Description;

            rhs.m_D3DResource = nullptr;
            rhs.m_SRVDescriptor = { 0 };
            rhs.m_SamplerDescriptor = { 0 };
        }

        return *this;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Texture::SetFilter(TextureFilter filter)
    {
        m_Description.Filter = filter;
        CreateSampler();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Texture::SetWrap(TextureWrap wrap)
    {
        m_Description.Wrap = wrap;
        CreateSampler();
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

    // -----------------------------------------------------------------------------------------------------------------------------
    const ClearValue& Texture::GetClearValue() const
    {
        return m_Description.ClearValue;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Texture::CreateSRV()
    {
        auto& dx12Device = Device::Get();

        if (m_SRVDescriptor.ptr)
            dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_SRVDescriptor, m_Type != TextureType::SwapChainBuffer);

        // Create SRV
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = Utils::AtomTextureFormatToSRVFormat(m_Description.Format);
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        if (m_Type == TextureType::TextureCube)
        {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
            srvDesc.TextureCube.MostDetailedMip = 0;
            srvDesc.TextureCube.MipLevels = m_Description.MipLevels;
        }
        else
        {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = m_Description.MipLevels;
        }

        m_SRVDescriptor = dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocateDescriptor();
        dx12Device.GetD3DDevice()->CreateShaderResourceView(m_D3DResource.Get(), &srvDesc, m_SRVDescriptor);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Texture::CreateSampler()
    {
        auto& dx12Device = Device::Get();

        if(m_SamplerDescriptor.ptr)
            dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::Sampler)->ReleaseDescriptor(m_SamplerDescriptor, m_Type != TextureType::SwapChainBuffer);

        D3D12_SAMPLER_DESC samplerDesc = {};
        samplerDesc.AddressU = Utils::AtomTextureWrapToD3D12(m_Description.Wrap);
        samplerDesc.AddressV = Utils::AtomTextureWrapToD3D12(m_Description.Wrap);
        samplerDesc.AddressW = Utils::AtomTextureWrapToD3D12(m_Description.Wrap);
        samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        samplerDesc.Filter = Utils::AtomTextureFilterToD3D12(m_Description.Filter);
        samplerDesc.MinLOD = 0.0f;
        samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
        samplerDesc.MaxAnisotropy = m_Description.Filter == TextureFilter::Anisotropic ? D3D12_REQ_MAXANISOTROPY : 1;

        m_SamplerDescriptor = dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::Sampler)->AllocateDescriptor();
        dx12Device.GetD3DDevice()->CreateSampler(&samplerDesc, m_SamplerDescriptor);
    }

    // ------------------------------------------------- Texture2D -----------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    Texture2D::Texture2D(const TextureDescription& description, const Vector<Vector<byte>>& pixelData, bool isReadable, const char* debugName)
        : Texture(TextureType::Texture2D, description, debugName), m_IsReadable(isReadable), Asset(AssetType::Texture2D)
    {
        // Copy the data if any is provided
        m_PixelData.resize(m_Description.MipLevels);
        for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
        {
            if (!pixelData[mip].empty())
            {
                Renderer::UploadTextureData(pixelData[mip].data(), this, mip);

                // For readable textures, store a copy of the data in CPU memory
                if (m_IsReadable)
                    m_PixelData[mip] = pixelData[mip];
            }
        }

        CreateUAV();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture2D::Texture2D(ID3D12Resource* textureHandle, const char* debugName)
        : Texture(TextureType::Texture2D, textureHandle, debugName), m_IsReadable(false), Asset(AssetType::Texture2D)
    {
        CreateUAV();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture2D::~Texture2D()
    {
        if (IsSet(m_Description.UsageFlags & TextureBindFlags::UnorderedAccess))
        {
            for (u32 i = 0; i < m_Description.MipLevels; i++)
            {
                Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_UAVDescriptors[i], true);
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture2D::Texture2D(Texture2D&& rhs) noexcept
        : Texture(std::move(rhs)), Asset(AssetType::Texture2D)
    {
        m_UAVDescriptors.resize(m_Description.MipLevels);

        for (u32 i = 0; i < m_Description.MipLevels; i++)
        {
            m_UAVDescriptors[i] = rhs.m_UAVDescriptors[i];
            rhs.m_UAVDescriptors[i] = { 0 };
        }

        m_PixelData.resize(m_Description.MipLevels);

        for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
        {
            m_PixelData[mip] = std::move(rhs.m_PixelData[mip]);
        }

        m_IsReadable = rhs.m_IsReadable;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture2D& Texture2D::operator=(Texture2D&& rhs) noexcept
    {
        if (this != &rhs)
        {
            if (IsSet(m_Description.UsageFlags & TextureBindFlags::UnorderedAccess))
            {
                for (u32 i = 0; i < m_Description.MipLevels; i++)
                {
                    Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_UAVDescriptors[i], true);
                }
            }

            Texture::operator=(std::move(rhs));

            m_UAVDescriptors.resize(m_Description.MipLevels);

            for (u32 i = 0; i < m_Description.MipLevels; i++)
            {
                m_UAVDescriptors[i] = rhs.m_UAVDescriptors[i];
                rhs.m_UAVDescriptors[i] = { 0 };
            }

            m_PixelData.resize(m_Description.MipLevels);

            for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
            {
                m_PixelData[mip] = std::move(rhs.m_PixelData[mip]);
            }

            m_IsReadable = rhs.m_IsReadable;
        }

        return *this;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Texture2D::UpdateGPUData(bool makeNonReadable)
    {
        if (m_IsReadable)
        {
            m_IsReadable = !makeNonReadable;

            for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
            {
                if (!m_PixelData.empty())
                {
                    Renderer::UploadTextureData(m_PixelData[mip].data(), this, mip);

                    if (makeNonReadable)
                        m_PixelData[mip].clear();
                }
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Texture2D::SetPixels(const Vector<byte>& pixels, u32 mipLevel)
    {
        if(m_IsReadable)
            m_PixelData[mipLevel] = pixels;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Vector<byte>& Texture2D::GetPixels(u32 mipLevel)
    {
        if (m_IsReadable)
            return m_PixelData[mipLevel];

        return {};
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool Texture2D::IsReadable() const
    {
        return m_IsReadable;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    D3D12_CPU_DESCRIPTOR_HANDLE Texture2D::GetUAV(u32 mip) const
    {
        ATOM_ENGINE_ASSERT(mip < m_Description.MipLevels); 
        ATOM_ENGINE_ASSERT(IsSet(m_Description.UsageFlags & TextureBindFlags::UnorderedAccess));
        return m_UAVDescriptors[mip];
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Texture2D::CreateUAV()
    {
        // Create UAVs
        if (IsSet(m_Description.UsageFlags & TextureBindFlags::UnorderedAccess))
        {
            auto& dx12Device = Device::Get();

            m_UAVDescriptors.resize(m_Description.MipLevels);
            for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
            {
                if(m_UAVDescriptors[mip].ptr)
                    dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_UAVDescriptors[mip], true);

                D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
                uavDesc.Format = Utils::AtomTextureFormatToUAVFormat(m_Description.Format);
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                uavDesc.Texture2D.MipSlice = mip;

                m_UAVDescriptors[mip] = dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocateDescriptor();
                dx12Device.GetD3DDevice()->CreateUnorderedAccessView(m_D3DResource.Get(), nullptr, &uavDesc, m_UAVDescriptors[mip]);
            }
        }
    }

    // ------------------------------------------------ RenderTexture2D ------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    RenderTexture2D::RenderTexture2D(const TextureDescription& description, bool swapChainBuffer, const char* debugName)
        : Texture(swapChainBuffer ? TextureType::SwapChainBuffer : TextureType::Texture2D, description, debugName)
    {
        m_Description.UsageFlags |= TextureBindFlags::RenderTarget;
        CreateRTV();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderTexture2D::RenderTexture2D(ID3D12Resource* textureHandle, bool swapChainBuffer, const char* debugName)
        : Texture(swapChainBuffer ? TextureType::SwapChainBuffer : TextureType::Texture2D, textureHandle, debugName)
    {
        m_Description.UsageFlags |= TextureBindFlags::RenderTarget;
        CreateRTV();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderTexture2D::~RenderTexture2D()
    {
        for (u32 i = 0; i < m_Description.MipLevels; i++)
        {
            Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::RenderTarget)->ReleaseDescriptor(m_RTVDescriptors[i], m_Type != TextureType::SwapChainBuffer);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderTexture2D::RenderTexture2D(RenderTexture2D&& rhs) noexcept
        : Texture(std::move(rhs))
    {
        m_RTVDescriptors.resize(m_Description.MipLevels);

        for (u32 i = 0; i < m_Description.MipLevels; i++)
        {
            m_RTVDescriptors[i] = rhs.m_RTVDescriptors[i];
            rhs.m_RTVDescriptors[i] = { 0 };
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderTexture2D& RenderTexture2D::operator=(RenderTexture2D&& rhs) noexcept
    {
        if (this != &rhs)
        {
            for (u32 i = 0; i < m_Description.MipLevels; i++)
            {
                Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::RenderTarget)->ReleaseDescriptor(m_RTVDescriptors[i], m_Type != TextureType::SwapChainBuffer);
            }

            Texture::operator=(std::move(rhs));

            m_RTVDescriptors.resize(m_Description.MipLevels);

            for (u32 i = 0; i < m_Description.MipLevels; i++)
            {
                m_RTVDescriptors[i] = rhs.m_RTVDescriptors[i];
                rhs.m_RTVDescriptors[i] = { 0 };
            }
        }

        return *this;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    D3D12_CPU_DESCRIPTOR_HANDLE RenderTexture2D::GetRTV(u32 mip) const
    {
        ATOM_ENGINE_ASSERT(mip < m_Description.MipLevels); 
        return m_RTVDescriptors[mip];
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderTexture2D::CreateRTV()
    {
        auto& dx12Device = Device::Get();

        // Create RTVs
        m_RTVDescriptors.resize(m_Description.MipLevels);
        for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
        {
            if(m_RTVDescriptors[mip].ptr)
                dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::RenderTarget)->ReleaseDescriptor(m_RTVDescriptors[mip], m_Type != TextureType::SwapChainBuffer);

            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = Utils::AtomTextureFormatToRTVFormat(m_Description.Format);
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice = mip;

            m_RTVDescriptors[mip] = dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::RenderTarget)->AllocateDescriptor();
            dx12Device.GetD3DDevice()->CreateRenderTargetView(m_D3DResource.Get(), &rtvDesc, m_RTVDescriptors[mip]);
        }
    }

    // --------------------------------------------------- DepthBuffer -------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    DepthBuffer::DepthBuffer(const TextureDescription& description, const char* debugName)
        : Texture(TextureType::Texture2D, description, debugName)
    {
        ATOM_ENGINE_ASSERT(m_Description.Format == TextureFormat::Depth24Stencil8 || m_Description.Format == TextureFormat::Depth32);
        m_Description.UsageFlags |= TextureBindFlags::DepthStencil;

        CreateDSV();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DepthBuffer::DepthBuffer(ID3D12Resource* textureHandle, const char* debugName)
        : Texture(TextureType::Texture2D, textureHandle, debugName)
    {
        ATOM_ENGINE_ASSERT(m_Description.Format == TextureFormat::Depth24Stencil8 || m_Description.Format == TextureFormat::Depth32);
        m_Description.UsageFlags |= TextureBindFlags::DepthStencil;

        CreateDSV();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DepthBuffer::~DepthBuffer()
    {
        for (u32 i = 0; i < m_Description.MipLevels; i++)
        {
            Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::DepthStencil)->ReleaseDescriptor(m_DSVDescriptors[i], true);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DepthBuffer::DepthBuffer(DepthBuffer&& rhs) noexcept
        : Texture(std::move(rhs))
    {
        m_DSVDescriptors.resize(m_Description.MipLevels);

        for (u32 i = 0; i < m_Description.MipLevels; i++)
        {
            m_DSVDescriptors[i] = rhs.m_DSVDescriptors[i];
            rhs.m_DSVDescriptors[i] = { 0 };
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DepthBuffer& DepthBuffer::operator=(DepthBuffer&& rhs) noexcept
    {
        if (this != &rhs)
        {
            for (u32 i = 0; i < m_Description.MipLevels; i++)
            {
                Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::DepthStencil)->ReleaseDescriptor(m_DSVDescriptors[i], true);
            }

            Texture::operator=(std::move(rhs));

            m_DSVDescriptors.resize(m_Description.MipLevels);

            for (u32 i = 0; i < m_Description.MipLevels; i++)
            {
                m_DSVDescriptors[i] = rhs.m_DSVDescriptors[i];
                rhs.m_DSVDescriptors[i] = { 0 };
            }
        }

        return *this;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    D3D12_CPU_DESCRIPTOR_HANDLE DepthBuffer::GetDSV(u32 mip) const
    {
        ATOM_ENGINE_ASSERT(mip < m_Description.MipLevels); 
        return m_DSVDescriptors[mip];
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DepthBuffer::CreateDSV()
    {
        auto& dx12Device = Device::Get();

        // Create DSVs
        m_DSVDescriptors.resize(m_Description.MipLevels);
        for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
        {
            if(m_DSVDescriptors[mip].ptr)
                dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::DepthStencil)->ReleaseDescriptor(m_DSVDescriptors[mip], true);

            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = Utils::AtomTextureFormatToDSVFormat(m_Description.Format);
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = mip;

            m_DSVDescriptors[mip] = dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::DepthStencil)->AllocateDescriptor();
            dx12Device.GetD3DDevice()->CreateDepthStencilView(m_D3DResource.Get(), &dsvDesc, m_DSVDescriptors[mip]);
        }
    }

    // --------------------------------------------------- TextureCube -------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    TextureCube::TextureCube(const TextureDescription& description, Vector<Vector<byte>> pixelData[6], bool isReadable, const char* debugName)
        : Texture(TextureType::TextureCube, description, debugName), m_IsReadable(isReadable), Asset(AssetType::TextureCube)
    {
        // Copy the data if any is provided
        for (u32 face = 0; face < 6; face++)
        {
            m_PixelData[face].resize(m_Description.MipLevels);

            for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
            {
                if (!pixelData[face][mip].empty())
                {
                    Renderer::UploadTextureData(pixelData[face][mip].data(), this, mip, face);

                    // For readable textures, store a copy of the data in CPU memory
                    if (m_IsReadable)
                        m_PixelData[face][mip] = pixelData[face][mip];
                }
            }
        }

        CreateUAV();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureCube::TextureCube(ID3D12Resource* textureHandle, const char* debugName)
        : Texture(TextureType::TextureCube, textureHandle, debugName), m_IsReadable(false), Asset(AssetType::TextureCube)
    {
        CreateUAV();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureCube::~TextureCube()
    {
        if (IsSet(m_Description.UsageFlags & TextureBindFlags::UnorderedAccess))
        {
            for (u32 i = 0; i < m_Description.MipLevels * 6; i++)
            {
                Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_UAVDescriptors[i], true);
            }

            for (u32 i = 0; i < m_Description.MipLevels; i++)
            {
                Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_ArrayUAVDescriptors[i], true);
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureCube::TextureCube(TextureCube&& rhs) noexcept
        : Texture(std::move(rhs)), Asset(AssetType::TextureCube)
    {
        m_UAVDescriptors.resize(m_Description.MipLevels * 6);

        for (u32 i = 0; i < m_Description.MipLevels * 6; i++)
        {
            m_UAVDescriptors[i] = rhs.m_UAVDescriptors[i];
            rhs.m_UAVDescriptors[i] = { 0 };
        }

        m_ArrayUAVDescriptors.resize(m_Description.MipLevels);

        for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
        {
            m_ArrayUAVDescriptors[mip] = rhs.m_ArrayUAVDescriptors[mip];
            rhs.m_ArrayUAVDescriptors[mip] = { 0 };
        }

        for (u32 face = 0; face < 6; face++)
        {
            m_PixelData[face].resize(m_Description.MipLevels);

            for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
            {
                m_PixelData[face][mip] = std::move(rhs.m_PixelData[face][mip]);
            }
        }

        m_IsReadable = rhs.m_IsReadable;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureCube& TextureCube::operator=(TextureCube&& rhs) noexcept
    {
        if (this != &rhs)
        {
            if (IsSet(m_Description.UsageFlags & TextureBindFlags::UnorderedAccess))
            {
                for (u32 i = 0; i < m_Description.MipLevels * 6; i++)
                {
                    Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_UAVDescriptors[i], true);
                }

                for (u32 i = 0; i < m_Description.MipLevels; i++)
                {
                    Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_ArrayUAVDescriptors[i], true);
                }
            }

            Texture::operator=(std::move(rhs));

            m_UAVDescriptors.resize(m_Description.MipLevels * 6);

            for (u32 i = 0; i < m_Description.MipLevels * 6; i++)
            {
                m_UAVDescriptors[i] = rhs.m_UAVDescriptors[i];
                rhs.m_UAVDescriptors[i] = { 0 };
            }

            m_ArrayUAVDescriptors.resize(m_Description.MipLevels);

            for (u32 i = 0; i < m_Description.MipLevels; i++)
            {
                m_ArrayUAVDescriptors[i] = rhs.m_ArrayUAVDescriptors[i];
                rhs.m_ArrayUAVDescriptors[i] = { 0 };
            }

            for (u32 face = 0; face < 6; face++)
            {
                m_PixelData[face].resize(m_Description.MipLevels);

                for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
                {
                    m_PixelData[face][mip] = std::move(rhs.m_PixelData[face][mip]);
                }
            }

            m_IsReadable = rhs.m_IsReadable;
        }

        return *this;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void TextureCube::UpdateGPUData(bool makeNonReadable)
    {
        if (m_IsReadable)
        {
            m_IsReadable = !makeNonReadable;

            for (u32 face = 0; face < 6; face++)
            {
                for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
                {
                    if (!m_PixelData[face][mip].empty())
                    {
                        Renderer::UploadTextureData(m_PixelData[face][mip].data(), this, mip, face);

                        if (makeNonReadable)
                            m_PixelData[face][mip].clear();
                    }
                }
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void TextureCube::SetPixels(const Vector<byte>& pixels, u32 cubeFace, u32 mipLevel)
    {
        ATOM_ENGINE_ASSERT(cubeFace < 6);

        if (m_IsReadable)
            m_PixelData[cubeFace][mipLevel] = pixels;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Vector<byte>& TextureCube::GetPixels(u32 cubeFace, u32 mipLevel)
    {
        ATOM_ENGINE_ASSERT(cubeFace < 6);

        if (m_IsReadable)
            return m_PixelData[cubeFace][mipLevel];

        return {};
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool TextureCube::IsReadable() const
    {
        return m_IsReadable;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    D3D12_CPU_DESCRIPTOR_HANDLE TextureCube::GetUAV(u32 slice, u32 mip) const
    {
        ATOM_ENGINE_ASSERT(mip < m_Description.MipLevels && slice < 6);
        ATOM_ENGINE_ASSERT(IsSet(m_Description.UsageFlags & TextureBindFlags::UnorderedAccess));
        u32 subresourceIdx = D3D12CalcSubresource(mip, slice, 0, m_Description.MipLevels, 6);
        return m_UAVDescriptors[subresourceIdx];
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    D3D12_CPU_DESCRIPTOR_HANDLE TextureCube::GetArrayUAV(u32 mip) const
    {
        ATOM_ENGINE_ASSERT(mip < m_Description.MipLevels);
        ATOM_ENGINE_ASSERT(IsSet(m_Description.UsageFlags & TextureBindFlags::UnorderedAccess));
        return m_ArrayUAVDescriptors[mip];
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void TextureCube::CreateUAV()
    {
        // Create UAVs
        if (IsSet(m_Description.UsageFlags & TextureBindFlags::UnorderedAccess))
        {
            auto& dx12Device = Device::Get();

            // Create descriptors for each individual subresource
            m_UAVDescriptors.resize(m_Description.MipLevels * 6);

            for (u32 slice = 0; slice < 6; slice++)
            {
                for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
                {
                    u32 subresourceIdx = D3D12CalcSubresource(mip, slice, 0, m_Description.MipLevels, 6);

                    if(m_UAVDescriptors[subresourceIdx].ptr)
                        dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_UAVDescriptors[subresourceIdx], true);

                    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
                    uavDesc.Format = Utils::AtomTextureFormatToUAVFormat(m_Description.Format);
                    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                    uavDesc.Texture2DArray.ArraySize = 1;
                    uavDesc.Texture2DArray.FirstArraySlice = slice;
                    uavDesc.Texture2DArray.MipSlice = mip;

                    m_UAVDescriptors[subresourceIdx] = dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocateDescriptor();
                    dx12Device.GetD3DDevice()->CreateUnorderedAccessView(m_D3DResource.Get(), nullptr, &uavDesc, m_UAVDescriptors[subresourceIdx]);
                }
            }

            // Create descriptors of the whole texture array for each mip level
            m_ArrayUAVDescriptors.resize(m_Description.MipLevels);

            for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
            {
                if(m_ArrayUAVDescriptors[mip].ptr)
                    dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_ArrayUAVDescriptors[mip], true);

                D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
                uavDesc.Format = Utils::AtomTextureFormatToUAVFormat(m_Description.Format);
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                uavDesc.Texture2DArray.ArraySize = 6;
                uavDesc.Texture2DArray.FirstArraySlice = 0;
                uavDesc.Texture2DArray.MipSlice = mip;

                m_ArrayUAVDescriptors[mip] = dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocateDescriptor();
                dx12Device.GetD3DDevice()->CreateUnorderedAccessView(m_D3DResource.Get(), nullptr, &uavDesc, m_ArrayUAVDescriptors[mip]);
            }
        }
    }
}
