#include "atompch.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Core/Application.h"

#include "Texture.h"
#include "Device.h"
#include "ResourceStateTracker.h"

#include "stb_image.h"
#include <glm/glm.hpp>

namespace Atom
{
    namespace Utils
    {
        u32 GetMaxMipCount(u32 width, u32 height)
        {
            return (u32)glm::log2((f32)glm::max(width, height)) + 1;
        }
    }

    // ------------------------------------------------------ Image2D --------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    Image2D::Image2D(u32 width, u32 height, u32 bytesPerPixel, u32 maxMipCount, bool isHDR, const byte* pixelData)
        : m_Width(width), m_Height(height), m_BytesPerPixel(bytesPerPixel), m_MaxMipCount(maxMipCount), m_IsHDR(isHDR)
    {
        m_PixelData.resize(m_Width * m_Height * m_BytesPerPixel);
        memcpy(m_PixelData.data(), pixelData, m_Width * m_Height * m_BytesPerPixel);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Image2D::Image2D(const std::filesystem::path& filepath, u32 desiredChannels)
    {
        String pathStr = filepath.string();
        m_IsHDR = stbi_is_hdr(pathStr.c_str());

        s32 width, height;
        byte* data = m_IsHDR ? (byte*)stbi_loadf(pathStr.c_str(), &width, &height, nullptr, desiredChannels) : stbi_load(pathStr.c_str(), &width, &height, nullptr, desiredChannels);
        ATOM_ENGINE_ASSERT(data, fmt::format("Failed to load texture file \"{}\"", filepath));

        m_Width = (u32)width;
        m_Height = (u32)height;
        m_BytesPerPixel = m_IsHDR ? desiredChannels * 4 : desiredChannels * 1;
        m_MaxMipCount = Utils::GetMaxMipCount(m_Width, m_Height);

        m_PixelData.resize(m_Width * m_Height * m_BytesPerPixel);
        memcpy(m_PixelData.data(), data, m_Width * m_Height * m_BytesPerPixel);

        stbi_image_free(data);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Image2D::Image2D(const byte* compressedData, u32 dataSize, u32 desiredChannels)
    {
        m_IsHDR = stbi_is_hdr_from_memory(compressedData, dataSize);

        s32 width, height;
        byte* decompressedData = m_IsHDR ? (byte*)stbi_loadf_from_memory(compressedData, dataSize, &width, &height, nullptr, desiredChannels) : stbi_load_from_memory(compressedData, dataSize, &width, &height, nullptr, desiredChannels);
        ATOM_ENGINE_ASSERT(decompressedData, "Failed to decompress texture!");

        m_Width = (u32)width;
        m_Height = (u32)height;
        m_BytesPerPixel = m_IsHDR ? desiredChannels * 4 : desiredChannels * 1;
        m_MaxMipCount = Utils::GetMaxMipCount(m_Width, m_Height);

        m_PixelData.resize(m_Width * m_Height * m_BytesPerPixel);
        memcpy(m_PixelData.data(), decompressedData, m_Width * m_Height * m_BytesPerPixel);

        stbi_image_free(decompressedData);
    }

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
            clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            clearValue.DepthStencil.Depth = m_Description.ClearValue.DepthStencil.DepthValue;
            clearValue.DepthStencil.Stencil = m_Description.ClearValue.DepthStencil.StencilValue;
        }
        else
        {
            clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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
        Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_SRVDescriptor, m_Type != TextureType::SwapChainBuffer);
        Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::Sampler)->ReleaseDescriptor(m_SamplerDescriptor, m_Type != TextureType::SwapChainBuffer);
        Device::Get().ReleaseResource(m_D3DResource.Detach(), m_Type != TextureType::SwapChainBuffer);
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
    void Texture::CreateViews()
    {
        auto& dx12Device = Device::Get();

        // Create SRV
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = Utils::AtomTextureFormatToSRVFormat(m_Description.Format);
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = m_Description.MipLevels;

        m_SRVDescriptor = dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocateDescriptor();
        dx12Device.GetD3DDevice()->CreateShaderResourceView(m_D3DResource.Get(), &srvDesc, m_SRVDescriptor);

        // Create Sampler
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
        if (IsSet(m_Description.UsageFlags & TextureBindFlags::UnorderedAccess))
        {
            for (u32 i = 0; i < m_Description.MipLevels; i++)
            {
                Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_UAVDescriptors[i], true);
            }
        }
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
        auto& dx12Device = Device::Get();

        Texture::CreateViews();

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
        for (u32 i = 0; i < m_Description.MipLevels; i++)
        {
            Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::RenderTarget)->ReleaseDescriptor(m_RTVDescriptors[i], m_Type != TextureType::SwapChainBuffer);
        }
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
        auto& dx12Device = Device::Get();

        Texture::CreateViews();

        // Create RTVs
        m_RTVDescriptors.resize(m_Description.MipLevels);
        for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
        {
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
        for (u32 i = 0; i < m_Description.MipLevels; i++)
        {
            Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::DepthStencil)->ReleaseDescriptor(m_DSVDescriptors[i], true);
        }
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
        auto& dx12Device = Device::Get();

        Texture::CreateViews();

        // Create DSVs
        m_DSVDescriptors.resize(m_Description.MipLevels);
        for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
        {
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = Utils::AtomTextureFormatToDSVFormat(m_Description.Format);
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = mip;

            m_DSVDescriptors[mip] = dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::DepthStencil)->AllocateDescriptor();
            dx12Device.GetD3DDevice()->CreateDepthStencilView(m_D3DResource.Get(), &dsvDesc, m_DSVDescriptors[mip]);
        }
    }
}
