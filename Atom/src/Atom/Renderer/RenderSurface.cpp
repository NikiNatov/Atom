#include "atompch.h"
#include "RenderSurface.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Renderer/Device.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurface::RenderSurface(const Ref<Texture>& texture, u32 mipIndex, u32 sliceIndex)
        : m_Texture(texture)
    {
        ATOM_ENGINE_ASSERT(m_Texture);

        CreateRTV(mipIndex, sliceIndex);
        CreateDSV(mipIndex, sliceIndex);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurface::~RenderSurface()
    {
        if (IsSet(m_Texture->GetFlags() & TextureFlags::RenderTarget))
        {
            Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::RenderTarget)->ReleaseDescriptor(m_RTVDescriptor, !IsSwapChainBuffer());
        }
        if (IsSet(m_Texture->GetFlags() & TextureFlags::DepthStencil))
        {
            Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::DepthStencil)->ReleaseDescriptor(m_DSVDescriptor, !IsSwapChainBuffer());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurface::RenderSurface(RenderSurface&& rhs) noexcept
        : m_Texture(std::move(rhs.m_Texture)), m_RTVDescriptor(rhs.m_RTVDescriptor), m_DSVDescriptor(rhs.m_DSVDescriptor)
    {
        rhs.m_Texture = nullptr;
        rhs.m_RTVDescriptor = { 0 };
        rhs.m_DSVDescriptor = { 0 };
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurface& RenderSurface::operator=(RenderSurface&& rhs) noexcept
    {
        if (this != &rhs)
        {
            if (IsSet(m_Texture->GetFlags() & TextureFlags::RenderTarget))
            {
                Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::RenderTarget)->ReleaseDescriptor(m_RTVDescriptor, !IsSwapChainBuffer());
            }
            if (IsSet(m_Texture->GetFlags() & TextureFlags::DepthStencil))
            {
                Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::DepthStencil)->ReleaseDescriptor(m_DSVDescriptor, !IsSwapChainBuffer());
            }

            m_Texture = std::move(rhs.m_Texture);
            m_RTVDescriptor = rhs.m_RTVDescriptor;
            m_DSVDescriptor = rhs.m_DSVDescriptor;
        }

        return *this;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureFormat RenderSurface::GetFormat() const
    {
        return m_Texture->GetFormat();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 RenderSurface::GetWidth() const
    {
        return m_Texture->GetWidth();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 RenderSurface::GetHeight() const
    {
        return m_Texture->GetHeight();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 RenderSurface::GetDepth() const
    {
        return m_Texture->GetDepth();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 RenderSurface::GetArraySize() const
    {
        return m_Texture->GetArraySize();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureFlags RenderSurface::GetFlags() const
    {
        return m_Texture->GetFlags();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const ClearValue& RenderSurface::GetClearValue() const
    {
        return m_Texture->GetClearValue();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Ref<Texture> RenderSurface::GetTexture() const
    {
        return m_Texture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool RenderSurface::IsSwapChainBuffer() const
    {
        return IsSet(m_Texture->GetFlags() & TextureFlags::SwapChainBuffer);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderSurface::CreateRTV(u32 mipIndex, u32 sliceIndex)
    {
        if (IsSet(m_Texture->GetFlags() & TextureFlags::RenderTarget))
        {
            ATOM_ENGINE_ASSERT(mipIndex < m_Texture->GetMipLevels());

            auto& dx12Device = Device::Get();

            if (m_RTVDescriptor.ptr)
                dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::RenderTarget)->ReleaseDescriptor(m_RTVDescriptor, !IsSwapChainBuffer());

            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = Utils::AtomTextureFormatToRTVFormat(m_Texture->GetFormat());

            if (m_Texture->GetType() == TextureType::Texture2D)
            {
                if (IsSet(m_Texture->GetFlags() & TextureFlags::CubeMap) || m_Texture->GetArraySize() > 1)
                {
                    u32 arraySizeMultiplier = IsSet(m_Texture->GetFlags() & TextureFlags::CubeMap) ? 6 : 1;

                    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                    rtvDesc.Texture2DArray.MipSlice = mipIndex;
                    rtvDesc.Texture2DArray.FirstArraySlice = sliceIndex == UINT32_MAX ? 0 : sliceIndex * arraySizeMultiplier;
                    rtvDesc.Texture2DArray.ArraySize = sliceIndex == UINT32_MAX ? m_Texture->GetArraySize() * arraySizeMultiplier : arraySizeMultiplier;
                }
                else
                {
                    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                    rtvDesc.Texture2D.MipSlice = mipIndex;
                }
            }
            else
            {
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                rtvDesc.Texture3D.MipSlice = mipIndex;
                rtvDesc.Texture3D.FirstWSlice = sliceIndex == UINT32_MAX ? 0 : sliceIndex;
                rtvDesc.Texture3D.WSize = sliceIndex == UINT32_MAX ? m_Texture->GetDepth() : 1;
            }

            m_RTVDescriptor = dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::RenderTarget)->AllocateDescriptor();
            dx12Device.GetD3DDevice()->CreateRenderTargetView(m_Texture->GetD3DResource().Get(), &rtvDesc, m_RTVDescriptor);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderSurface::CreateDSV(u32 mipIndex, u32 sliceIndex)
    {
        if (IsSet(m_Texture->GetFlags() & TextureFlags::DepthStencil))
        {
            ATOM_ENGINE_ASSERT(m_Texture->GetType() == TextureType::Texture2D);
            ATOM_ENGINE_ASSERT(mipIndex < m_Texture->GetMipLevels());

            auto& dx12Device = Device::Get();

            if (m_DSVDescriptor.ptr)
                dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::DepthStencil)->ReleaseDescriptor(m_DSVDescriptor, !IsSwapChainBuffer());

            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = Utils::AtomTextureFormatToDSVFormat(m_Texture->GetFormat());

            if (IsSet(m_Texture->GetFlags() & TextureFlags::CubeMap) || m_Texture->GetArraySize() > 1)
            {
                u32 arraySizeMultiplier = IsSet(m_Texture->GetFlags() & TextureFlags::CubeMap) ? 6 : 1;

                dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                dsvDesc.Texture2DArray.MipSlice = mipIndex;
                dsvDesc.Texture2DArray.FirstArraySlice = sliceIndex == UINT32_MAX ? 0 : sliceIndex * arraySizeMultiplier;
                dsvDesc.Texture2DArray.ArraySize = sliceIndex == UINT32_MAX ? m_Texture->GetArraySize() * arraySizeMultiplier : arraySizeMultiplier;
            }
            else
            {
                dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                dsvDesc.Texture2D.MipSlice = mipIndex;
            }

            m_DSVDescriptor = dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::DepthStencil)->AllocateDescriptor();
            dx12Device.GetD3DDevice()->CreateDepthStencilView(m_Texture->GetD3DResource().Get(), &dsvDesc, m_DSVDescriptor);
        }
    }

}
