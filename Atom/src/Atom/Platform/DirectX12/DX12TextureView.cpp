#include "atompch.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DX12TextureView.h"
#include "DX12Texture.h"
#include "DX12Device.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    DX12TextureViewSR::DX12TextureViewSR(const Ref<Texture>& texture)
        : m_Texture(texture.get())
    {
        ATOM_ENGINE_ASSERT(m_Texture->GetUsageFlags() & TextureUsage::ShaderResource, "Texture is missing the ShaderResource flag!");

        auto dx12Device = Renderer::GetDevice().As<DX12Device>();

        // Create description
        m_Description.Format = Utils::AtomTextureFormatToSRVFormat(m_Texture->GetFormat());
        m_Description.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        switch (m_Texture->GetType())
        {
            case TextureType::Texture2D:
            case TextureType::SwapChainBuffer:
            {
                m_Description.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                m_Description.Texture2D.MostDetailedMip = 0;
                m_Description.Texture2D.MipLevels = m_Texture->GetMipLevels();
                break;
            }
            case TextureType::TextureCube:
            {
                m_Description.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                m_Description.TextureCube.MostDetailedMip = 0;
                m_Description.TextureCube.MipLevels = m_Texture->GetMipLevels();
                break;
            }
        }

        // Allocate descriptor and create view
        m_Descriptor = dx12Device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        dx12Device->GetD3DDevice()->CreateShaderResourceView(m_Texture->As<DX12Texture>()->GetD3DResource().Get(), &m_Description, m_Descriptor.GetCPUHandle());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12TextureViewSR::~DX12TextureViewSR()
    {
        if (m_Descriptor.IsValid())
            m_Descriptor.DeferredRelease();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Texture* DX12TextureViewSR::GetTextureResource() const
    {
        return m_Texture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12TextureViewRT::DX12TextureViewRT(const Ref<Texture>& texture, u32 mipLevel, u32 cubeFace)
        : m_Texture(texture.get()), m_MipLevel(mipLevel), m_CubeFace(cubeFace)
    {
        ATOM_ENGINE_ASSERT(m_Texture->GetUsageFlags() & TextureUsage::RenderTarget, "Texture is missing the RenderTarget flag!");
        ATOM_ENGINE_ASSERT(mipLevel < texture->GetMipLevels());

        auto dx12Device = Renderer::GetDevice().As<DX12Device>();

        // Create description
        m_Description.Format = Utils::AtomTextureFormatToRTVFormat(m_Texture->GetFormat());

        switch (m_Texture->GetType())
        {
            case TextureType::Texture2D:
            case TextureType::SwapChainBuffer:
            {
                ATOM_ENGINE_ASSERT(cubeFace == UINT32_MAX, "Texture is not a cube map!");

                m_Description.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                m_Description.Texture2D.MipSlice = mipLevel;
                break;
            }
            case TextureType::TextureCube:
            {
                m_Description.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                m_Description.Texture2DArray.MipSlice = mipLevel;
                m_Description.Texture2DArray.FirstArraySlice = cubeFace == UINT32_MAX ? 0 : cubeFace;
                m_Description.Texture2DArray.ArraySize = cubeFace == UINT32_MAX ? 6 : 1;
                break;
            }
        }

        // Allocate descriptor
        m_Descriptor = dx12Device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        dx12Device->GetD3DDevice()->CreateRenderTargetView(m_Texture->As<DX12Texture>()->GetD3DResource().Get(), &m_Description, m_Descriptor.GetCPUHandle());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12TextureViewRT::~DX12TextureViewRT()
    {
        if(m_Descriptor.IsValid())
            m_Descriptor.DeferredRelease();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Texture* DX12TextureViewRT::GetTextureResource() const
    {
        return m_Texture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12TextureViewRT::GetMipLevel() const
    {
        return m_MipLevel;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12TextureViewRT::GetCubeFace() const
    {
        return m_CubeFace;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12TextureViewDS::DX12TextureViewDS(const Ref<Texture>& texture, u32 mipLevel, u32 cubeFace)
        : m_Texture(texture.get()), m_MipLevel(mipLevel), m_CubeFace(cubeFace)
    {
        ATOM_ENGINE_ASSERT(m_Texture->GetUsageFlags() & TextureUsage::DepthBuffer, "Texture is missing the DepthBuffer flag!");
        ATOM_ENGINE_ASSERT(mipLevel < texture->GetMipLevels());

        auto dx12Device = Renderer::GetDevice().As<DX12Device>();

        // Create description
        m_Description.Format = Utils::AtomTextureFormatToDSVFormat(m_Texture->GetFormat());

        switch (m_Texture->GetType())
        {
            case TextureType::Texture2D:
            case TextureType::SwapChainBuffer:
            {
                ATOM_ENGINE_ASSERT(cubeFace == UINT32_MAX, "Texture is not a cube map!");

                m_Description.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                m_Description.Texture2D.MipSlice = mipLevel;
                break;
            }
            case TextureType::TextureCube:
            {
                m_Description.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                m_Description.Texture2DArray.MipSlice = mipLevel;
                m_Description.Texture2DArray.FirstArraySlice = cubeFace == UINT32_MAX ? 0 : cubeFace;
                m_Description.Texture2DArray.ArraySize = cubeFace == UINT32_MAX ? 6 : 1;
                break;
            }
        }

        // Allocate descriptor
        m_Descriptor = dx12Device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        dx12Device->GetD3DDevice()->CreateDepthStencilView(m_Texture->As<DX12Texture>()->GetD3DResource().Get(), &m_Description, m_Descriptor.GetCPUHandle());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12TextureViewDS::~DX12TextureViewDS()
    {
        if (m_Descriptor.IsValid())
            m_Descriptor.DeferredRelease();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Texture* DX12TextureViewDS::GetTextureResource() const
    {
        return m_Texture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12TextureViewDS::GetMipLevel() const
    {
        return m_MipLevel;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12TextureViewDS::GetCubeFace() const
    {
        return m_CubeFace;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12TextureViewUA::DX12TextureViewUA(const Ref<Texture>& texture, u32 mipLevel, u32 cubeFace)
        : m_Texture(texture.get()), m_MipLevel(mipLevel), m_CubeFace(cubeFace)
    {
        ATOM_ENGINE_ASSERT(m_Texture->GetUsageFlags() & TextureUsage::UnorderedAccess, "Texture is missing the UnorderedAccess flag!");
        ATOM_ENGINE_ASSERT(mipLevel < texture->GetMipLevels());

        auto dx12Device = Renderer::GetDevice().As<DX12Device>();

        // Create description
        m_Description.Format = Utils::AtomTextureFormatToD3D12(m_Texture->GetFormat());

        switch (m_Texture->GetType())
        {
            case TextureType::Texture2D:
            case TextureType::SwapChainBuffer:
            {
                ATOM_ENGINE_ASSERT(cubeFace == UINT32_MAX, "Texture is not a cube map!");

                m_Description.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                m_Description.Texture2D.MipSlice = mipLevel;
                break;
            }
            case TextureType::TextureCube:
            {
                m_Description.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                m_Description.Texture2DArray.MipSlice = mipLevel;
                m_Description.Texture2DArray.FirstArraySlice = cubeFace == UINT32_MAX ? 0 : cubeFace;
                m_Description.Texture2DArray.ArraySize = cubeFace == UINT32_MAX ? 6 : 1;
                break;
            }
        }

        // Allocate descriptor
        m_Descriptor = dx12Device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        dx12Device->GetD3DDevice()->CreateUnorderedAccessView(m_Texture->As<DX12Texture>()->GetD3DResource().Get(), nullptr, &m_Description, m_Descriptor.GetCPUHandle());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12TextureViewUA::~DX12TextureViewUA()
    {
        if (m_Descriptor.IsValid())
            m_Descriptor.DeferredRelease();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Texture* DX12TextureViewUA::GetTextureResource() const
    {
        return m_Texture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12TextureViewUA::GetMipLevel() const
    {
        return m_MipLevel;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12TextureViewUA::GetCubeFace() const
    {
        return m_CubeFace;
    }

}

#endif // ATOM_PLATFORM_WINDOWS