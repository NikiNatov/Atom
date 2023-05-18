#include "atompch.h"
#include "Texture.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Core/Application.h"

#include "Atom/Renderer/Device.h"
#include "Atom/Renderer/ResourceStateTracker.h"

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

        DXCall(d3dDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON,
            isRenderTarget || isDepthStencil ? &clearValue : nullptr, IID_PPV_ARGS(&m_D3DResource)));

#if defined (ATOM_DEBUG)
        String name = debugName;
        DXCall(m_D3DResource->SetName(STRING_TO_WSTRING(name).c_str()));
#endif

        ResourceStateTracker::AddGlobalResourceState(m_D3DResource.Get(), D3D12_RESOURCE_STATE_COMMON);

        // Create views
        CreateSRV();
        CreateUAV(0);
        CreateSampler();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture::Texture(const Texture& aliasedTexture, u32 mipIndex, u32 sliceIndex, const char* debugName)
        : m_Description(aliasedTexture.m_Description), m_D3DResource(aliasedTexture.m_D3DResource), m_IsAlias(true)
    {
        m_Description.Width = mipIndex == UINT32_MAX ? aliasedTexture.m_Description.Width : aliasedTexture.m_Description.Width >> mipIndex;
        m_Description.Height = mipIndex == UINT32_MAX ? aliasedTexture.m_Description.Height : aliasedTexture.m_Description.Height >> mipIndex;

        if(m_Description.Type == TextureType::Texture3D)
            m_Description.Depth = mipIndex == UINT32_MAX ? aliasedTexture.m_Description.Depth : aliasedTexture.m_Description.Depth >> mipIndex;
        else
            m_Description.ArraySize = sliceIndex == UINT32_MAX ? aliasedTexture.m_Description.ArraySize : 1;

        m_Description.MipLevels = mipIndex == UINT32_MAX ? aliasedTexture.m_Description.MipLevels : 1;

#if defined (ATOM_DEBUG)
        String name = debugName;
        DXCall(m_D3DResource->SetName(STRING_TO_WSTRING(name).c_str()));
#endif

        CreateSRV(mipIndex, sliceIndex);
        CreateUAV(mipIndex, sliceIndex);
        CreateSampler();
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
        m_Description.Filter = TextureFilter::Linear;
        m_Description.Wrap = TextureWrap::Repeat;

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
        CreateSRV();
        CreateUAV(0);
        CreateSampler();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture::~Texture()
    {
        if (IsSet(m_Description.Flags & TextureFlags::ShaderResource))
        {
            Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_SRVDescriptor, !IsSet(m_Description.Flags & TextureFlags::SwapChainBuffer));
            Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::Sampler)->ReleaseDescriptor(m_SamplerDescriptor, !IsSet(m_Description.Flags & TextureFlags::SwapChainBuffer));
        }

        if (IsSet(m_Description.Flags & TextureFlags::UnorderedAccess))
        {
            Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_UAVDescriptor, !IsSet(m_Description.Flags & TextureFlags::SwapChainBuffer));
        }

        if(!m_IsAlias)
            Device::Get().ReleaseResource(m_D3DResource.Detach(), !IsSet(m_Description.Flags & TextureFlags::SwapChainBuffer));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture::Texture(Texture&& rhs) noexcept
        : m_D3DResource(rhs.m_D3DResource), m_SRVDescriptor(rhs.m_SRVDescriptor), m_SamplerDescriptor(rhs.m_SamplerDescriptor), m_Description(rhs.m_Description), m_IsAlias(rhs.m_IsAlias)
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
            if (IsSet(m_Description.Flags & TextureFlags::ShaderResource))
            {
                Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_SRVDescriptor, !IsSet(m_Description.Flags & TextureFlags::SwapChainBuffer));
                Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::Sampler)->ReleaseDescriptor(m_SamplerDescriptor, !IsSet(m_Description.Flags & TextureFlags::SwapChainBuffer));
            }

            if (IsSet(m_Description.Flags & TextureFlags::UnorderedAccess))
            {
                Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_UAVDescriptor, !IsSet(m_Description.Flags & TextureFlags::SwapChainBuffer));
            }

            Device::Get().ReleaseResource(m_D3DResource.Detach(), !IsSet(m_Description.Flags & TextureFlags::SwapChainBuffer));

            m_D3DResource = rhs.m_D3DResource;
            m_SRVDescriptor = rhs.m_SRVDescriptor;
            m_SamplerDescriptor = rhs.m_SamplerDescriptor;
            m_Description = rhs.m_Description;
            m_IsAlias = rhs.m_IsAlias;

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
    const TextureDescription& Texture::GetDescription() const
    {
        return m_Description;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const ClearValue& Texture::GetClearValue() const
    {
        return m_Description.ClearValue;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Texture::CalculateMaxMipCount(u32 width, u32 height)
    {
        return (u32)glm::log2((f32)glm::max(width, height)) + 1;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Texture::CalculateSubresource(u32 mip, u32 mipCount, u32 slice, u32 arraySize)
    {
        return D3D12CalcSubresource(mip, slice, 0, mipCount, arraySize);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Texture::CreateSRV(u32 mipIndex, u32 sliceIndex)
    {
        if (IsSet(m_Description.Flags & TextureFlags::ShaderResource))
        {
            auto& dx12Device = Device::Get();

            if (m_SRVDescriptor.ptr)
                dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_SRVDescriptor, !IsSet(m_Description.Flags & TextureFlags::SwapChainBuffer));

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = Utils::AtomTextureFormatToSRVFormat(m_Description.Format);
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

            if (m_Description.Type == TextureType::Texture2D)
            {
                if (IsSet(m_Description.Flags & TextureFlags::CubeMap))
                {
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                    srvDesc.TextureCube.MostDetailedMip = mipIndex == UINT32_MAX ? 0 : mipIndex;
                    srvDesc.TextureCube.MipLevels = mipIndex == UINT32_MAX ? m_Description.MipLevels : 1;
                }
                else
                {
                    if (m_Description.ArraySize > 1)
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                        srvDesc.Texture2DArray.MostDetailedMip = mipIndex == UINT32_MAX ? 0 : mipIndex;
                        srvDesc.Texture2DArray.MipLevels = mipIndex == UINT32_MAX ? m_Description.MipLevels : 1;
                        srvDesc.Texture2DArray.FirstArraySlice = sliceIndex == UINT32_MAX ? 0 : sliceIndex;
                        srvDesc.Texture2DArray.ArraySize = sliceIndex == UINT32_MAX ? m_Description.ArraySize : 1;
                    }
                    else
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                        srvDesc.Texture2D.MostDetailedMip = mipIndex == UINT32_MAX ? 0 : mipIndex;
                        srvDesc.Texture2D.MipLevels = mipIndex == UINT32_MAX ? m_Description.MipLevels : 1;
                    }
                }
            }
            else
            {
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
                srvDesc.Texture3D.MostDetailedMip = mipIndex == UINT32_MAX ? 0 : mipIndex;
                srvDesc.Texture3D.MipLevels = mipIndex == UINT32_MAX ? m_Description.MipLevels : 1;
            }

            m_SRVDescriptor = dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocateDescriptor();
            dx12Device.GetD3DDevice()->CreateShaderResourceView(m_D3DResource.Get(), &srvDesc, m_SRVDescriptor);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Texture::CreateUAV(u32 mipIndex, u32 sliceIndex)
    {
        if (IsSet(m_Description.Flags & TextureFlags::UnorderedAccess))
        {
            auto& dx12Device = Device::Get();

            if (m_UAVDescriptor.ptr)
                dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_UAVDescriptor, !IsSet(m_Description.Flags & TextureFlags::SwapChainBuffer));

            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = Utils::AtomTextureFormatToUAVFormat(m_Description.Format);

            if (m_Description.Type == TextureType::Texture2D)
            {
                if (IsSet(m_Description.Flags & TextureFlags::CubeMap) || m_Description.ArraySize > 1)
                {
                    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                    uavDesc.Texture2DArray.MipSlice = mipIndex;
                    uavDesc.Texture2DArray.FirstArraySlice = sliceIndex == UINT32_MAX ? 0 : sliceIndex;
                    uavDesc.Texture2DArray.ArraySize = sliceIndex == UINT32_MAX ? m_Description.ArraySize : 1;
                }
                else
                {
                    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                    uavDesc.Texture2D.MipSlice = mipIndex;
                }
            }
            else
            {
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
                uavDesc.Texture3D.MipSlice = mipIndex;
                uavDesc.Texture3D.FirstWSlice = sliceIndex == UINT32_MAX ? 0 : sliceIndex;
                uavDesc.Texture3D.WSize = sliceIndex == UINT32_MAX ? m_Description.Depth : 1;
            }

            m_UAVDescriptor = dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocateDescriptor();
            dx12Device.GetD3DDevice()->CreateUnorderedAccessView(m_D3DResource.Get(), nullptr, &uavDesc, m_UAVDescriptor);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Texture::CreateSampler()
    {
        if (IsSet(m_Description.Flags & TextureFlags::ShaderResource))
        {
            auto& dx12Device = Device::Get();

            if (m_SamplerDescriptor.ptr)
                dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::Sampler)->ReleaseDescriptor(m_SamplerDescriptor, !IsSet(m_Description.Flags & TextureFlags::SwapChainBuffer));

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
    }
}
