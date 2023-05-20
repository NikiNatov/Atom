#include "atompch.h"
#include "TextureView.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Renderer/Device.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/RenderSurface.h"

namespace Atom
{
    // ------------------------------------------------------ TextureViewRO --------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewRO::TextureViewRO(const Texture* resource, const TextureViewDescription& description, bool deferredRelease)
        : m_DeferredRelease(deferredRelease)
    {
        auto& dx12Device = Device::Get();

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = Utils::AtomTextureFormatToSRVFormat(resource->GetFormat());
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        if (resource->GetType() == TextureType::Texture2D)
        {
            if (IsSet(resource->GetFlags() & TextureFlags::CubeMap))
            {
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                srvDesc.TextureCube.MostDetailedMip = description.FirstMip;
                srvDesc.TextureCube.MipLevels = description.MipLevels;
            }
            else
            {
                if (description.ArraySize > 1)
                {
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                    srvDesc.Texture2DArray.MostDetailedMip = description.FirstMip;
                    srvDesc.Texture2DArray.MipLevels = description.MipLevels;
                    srvDesc.Texture2DArray.FirstArraySlice = description.FirstSlice;
                    srvDesc.Texture2DArray.ArraySize = description.ArraySize;
                }
                else
                {
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                    srvDesc.Texture2D.MostDetailedMip = description.FirstMip;
                    srvDesc.Texture2D.MipLevels = description.MipLevels;
                }
            }
        }
        else
        {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            srvDesc.Texture3D.MostDetailedMip = description.FirstMip;
            srvDesc.Texture3D.MipLevels = description.MipLevels;
        }

        m_Descriptor = dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocateDescriptor();
        dx12Device.GetD3DDevice()->CreateShaderResourceView(resource->GetD3DResource().Get(), &srvDesc, m_Descriptor);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewRO::~TextureViewRO()
    {
        Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_Descriptor, m_DeferredRelease);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewRO::TextureViewRO(TextureViewRO&& rhs) noexcept
        : m_Descriptor(std::move(rhs.m_Descriptor)), m_DeferredRelease(rhs.m_DeferredRelease)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewRO& TextureViewRO::operator=(TextureViewRO&& rhs) noexcept
    {
        if (this != &rhs)
        {
            Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_Descriptor, m_DeferredRelease);

            m_Descriptor = std::move(rhs.m_Descriptor);
            m_DeferredRelease = rhs.m_DeferredRelease;
        }

        return *this;
    }

    // ------------------------------------------------------ TextureViewRW --------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewRW::TextureViewRW(const Texture* resource, const TextureViewDescription& description, bool deferredRelease)
        : m_DeferredRelease(deferredRelease)
    {
        auto& dx12Device = Device::Get();

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = Utils::AtomTextureFormatToUAVFormat(resource->GetFormat());

        if (resource->GetType() == TextureType::Texture2D)
        {
            if (IsSet(resource->GetFlags() & TextureFlags::CubeMap) || description.ArraySize > 1)
            {
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                uavDesc.Texture2DArray.MipSlice = description.FirstMip;
                uavDesc.Texture2DArray.FirstArraySlice = description.FirstSlice;
                uavDesc.Texture2DArray.ArraySize = description.ArraySize;
            }
            else
            {
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                uavDesc.Texture2D.MipSlice = description.FirstMip;
            }
        }
        else
        {
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
            uavDesc.Texture3D.MipSlice = description.FirstMip;
            uavDesc.Texture3D.FirstWSlice = description.FirstSlice;
            uavDesc.Texture3D.WSize = description.ArraySize;
        }

        m_Descriptor = dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocateDescriptor();
        dx12Device.GetD3DDevice()->CreateUnorderedAccessView(resource->GetD3DResource().Get(), nullptr, &uavDesc, m_Descriptor);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewRW::~TextureViewRW()
    {
        Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_Descriptor, m_DeferredRelease);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewRW::TextureViewRW(TextureViewRW&& rhs) noexcept
        : m_Descriptor(std::move(rhs.m_Descriptor)), m_DeferredRelease(rhs.m_DeferredRelease)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewRW& TextureViewRW::operator=(TextureViewRW&& rhs) noexcept
    {
        if (this != &rhs)
        {
            Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_Descriptor, m_DeferredRelease);

            m_Descriptor = std::move(rhs.m_Descriptor);
            m_DeferredRelease = rhs.m_DeferredRelease;
        }

        return *this;
    }

    // ------------------------------------------------------ TextureViewRT --------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewRT::TextureViewRT(const RenderSurface* resource, const TextureViewDescription& description, bool deferredRelease)
        : m_DeferredRelease(deferredRelease)
    {
        auto& dx12Device = Device::Get();
        
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = Utils::AtomTextureFormatToRTVFormat(resource->GetFormat());

        if (resource->GetTexture()->GetType() == TextureType::Texture2D)
        {
            if (IsSet(resource->GetFlags() & TextureFlags::CubeMap) || description.ArraySize > 1)
            {
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                rtvDesc.Texture2DArray.MipSlice = description.FirstMip;
                rtvDesc.Texture2DArray.FirstArraySlice = description.FirstSlice;
                rtvDesc.Texture2DArray.ArraySize = description.ArraySize;
            }
            else
            {
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice = description.FirstMip;
            }
        }
        else
        {
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
            rtvDesc.Texture3D.MipSlice = description.FirstMip;
            rtvDesc.Texture3D.FirstWSlice = description.FirstSlice;
            rtvDesc.Texture3D.WSize = description.ArraySize;
        }

        m_Descriptor = dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::RenderTarget)->AllocateDescriptor();
        dx12Device.GetD3DDevice()->CreateRenderTargetView(resource->GetTexture()->GetD3DResource().Get(), &rtvDesc, m_Descriptor);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewRT::~TextureViewRT()
    {
        Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::RenderTarget)->ReleaseDescriptor(m_Descriptor, m_DeferredRelease);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewRT::TextureViewRT(TextureViewRT&& rhs) noexcept
        : m_Descriptor(std::move(rhs.m_Descriptor)), m_DeferredRelease(rhs.m_DeferredRelease)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewRT& TextureViewRT::operator=(TextureViewRT&& rhs) noexcept
    {
        if (this != &rhs)
        {
            Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::RenderTarget)->ReleaseDescriptor(m_Descriptor, m_DeferredRelease);

            m_Descriptor = std::move(rhs.m_Descriptor);
            m_DeferredRelease = rhs.m_DeferredRelease;
        }

        return *this;
    }

    // ------------------------------------------------------ TextureViewDS --------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewDS::TextureViewDS(const RenderSurface* resource, const TextureViewDescription& description, bool deferredRelease)
        : m_DeferredRelease(deferredRelease)
    {
        ATOM_ENGINE_ASSERT(resource->GetTexture()->GetType() != TextureType::Texture3D);

        auto& dx12Device = Device::Get();

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = Utils::AtomTextureFormatToDSVFormat(resource->GetFormat());

        if (IsSet(resource->GetFlags() & TextureFlags::CubeMap) || description.ArraySize > 1)
        {
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
            dsvDesc.Texture2DArray.MipSlice = description.FirstMip;
            dsvDesc.Texture2DArray.FirstArraySlice = description.FirstSlice;
            dsvDesc.Texture2DArray.ArraySize = description.ArraySize;
        }
        else
        {
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = description.FirstMip;
        }

        m_Descriptor = dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::DepthStencil)->AllocateDescriptor();
        dx12Device.GetD3DDevice()->CreateDepthStencilView(resource->GetTexture()->GetD3DResource().Get(), &dsvDesc, m_Descriptor);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewDS::~TextureViewDS()
    {
        Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::DepthStencil)->ReleaseDescriptor(m_Descriptor, m_DeferredRelease);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewDS::TextureViewDS(TextureViewDS&& rhs) noexcept
        : m_Descriptor(std::move(rhs.m_Descriptor)), m_DeferredRelease(rhs.m_DeferredRelease)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureViewDS& TextureViewDS::operator=(TextureViewDS&& rhs) noexcept
    {
        if (this != &rhs)
        {
            Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::DepthStencil)->ReleaseDescriptor(m_Descriptor, m_DeferredRelease);

            m_Descriptor = std::move(rhs.m_Descriptor);
            m_DeferredRelease = rhs.m_DeferredRelease;
        }

        return *this;
    }
}
