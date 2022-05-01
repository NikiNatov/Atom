#include "atompch.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"

#include "TextureView.h"
#include "Texture.h"
#include "Device.h"
#include "Renderer.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewSR::TextureViewSR(const Ref<Texture>& texture)
        : m_Texture(texture.get())
    {
        ATOM_ENGINE_ASSERT(m_Texture->GetUsageFlags() & TextureUsage::ShaderResource, "Texture is missing the ShaderResource flag!");

        auto dx12Device = Renderer::GetDevice();

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
        m_Descriptor = dx12Device->GetCPUDescriptorHeap(DescriptorHeapType::ShaderResources)->AllocateDescriptor();
        dx12Device->GetD3DDevice()->CreateShaderResourceView(m_Texture->GetD3DResource().Get(), &m_Description, m_Descriptor);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewSR::~TextureViewSR()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Texture* TextureViewSR::GetTextureResource() const
    {
        return m_Texture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewRT::TextureViewRT(const Ref<Texture>& texture, u32 mipLevel, u32 cubeFace)
        : m_Texture(texture.get()), m_MipLevel(mipLevel), m_CubeFace(cubeFace)
    {
        ATOM_ENGINE_ASSERT(m_Texture->GetUsageFlags() & TextureUsage::RenderTarget, "Texture is missing the RenderTarget flag!");
        ATOM_ENGINE_ASSERT(mipLevel < texture->GetMipLevels());

        auto dx12Device = Renderer::GetDevice();

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
        m_Descriptor = dx12Device->GetCPUDescriptorHeap(DescriptorHeapType::RenderTargets)->AllocateDescriptor();
        dx12Device->GetD3DDevice()->CreateRenderTargetView(m_Texture->GetD3DResource().Get(), &m_Description, m_Descriptor);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewRT::~TextureViewRT()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Texture* TextureViewRT::GetTextureResource() const
    {
        return m_Texture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 TextureViewRT::GetMipLevel() const
    {
        return m_MipLevel;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 TextureViewRT::GetCubeFace() const
    {
        return m_CubeFace;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewDS::TextureViewDS(const Ref<Texture>& texture, u32 mipLevel, u32 cubeFace)
        : m_Texture(texture.get()), m_MipLevel(mipLevel), m_CubeFace(cubeFace)
    {
        ATOM_ENGINE_ASSERT(m_Texture->GetUsageFlags() & TextureUsage::DepthBuffer, "Texture is missing the DepthBuffer flag!");
        ATOM_ENGINE_ASSERT(mipLevel < texture->GetMipLevels());

        auto dx12Device = Renderer::GetDevice();

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
        m_Descriptor = dx12Device->GetCPUDescriptorHeap(DescriptorHeapType::DepthStencils)->AllocateDescriptor();
        dx12Device->GetD3DDevice()->CreateDepthStencilView(m_Texture->GetD3DResource().Get(), &m_Description, m_Descriptor);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewDS::~TextureViewDS()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Texture* TextureViewDS::GetTextureResource() const
    {
        return m_Texture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 TextureViewDS::GetMipLevel() const
    {
        return m_MipLevel;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 TextureViewDS::GetCubeFace() const
    {
        return m_CubeFace;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewUA::TextureViewUA(const Ref<Texture>& texture, u32 mipLevel, u32 cubeFace)
        : m_Texture(texture.get()), m_MipLevel(mipLevel), m_CubeFace(cubeFace)
    {
        ATOM_ENGINE_ASSERT(m_Texture->GetUsageFlags() & TextureUsage::UnorderedAccess, "Texture is missing the UnorderedAccess flag!");
        ATOM_ENGINE_ASSERT(mipLevel < texture->GetMipLevels());

        auto dx12Device = Renderer::GetDevice();

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
        m_Descriptor = dx12Device->GetCPUDescriptorHeap(DescriptorHeapType::ShaderResources)->AllocateDescriptor();
        dx12Device->GetD3DDevice()->CreateUnorderedAccessView(m_Texture->GetD3DResource().Get(), nullptr, &m_Description, m_Descriptor);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewUA::~TextureViewUA()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Texture* TextureViewUA::GetTextureResource() const
    {
        return m_Texture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 TextureViewUA::GetMipLevel() const
    {
        return m_MipLevel;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 TextureViewUA::GetCubeFace() const
    {
        return m_CubeFace;
    }

}
