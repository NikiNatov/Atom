#pragma once

#include "Atom/Renderer/API/Texture.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"
#include "DX12DescriptorAllocator.h"

namespace Atom
{
    class DX12Texture2D : public Texture2D
    {
    public:
        DX12Texture2D(const TextureDescription& description);
        DX12Texture2D(const String& filename, const TextureDescription& description);
        DX12Texture2D(DX12Device& device, wrl::ComPtr<ID3D12Resource2> resource, TextureFilter filter, TextureWrap wrap);
        ~DX12Texture2D();

        virtual void Release() override;
        virtual void DeferredRelease() override;
        virtual void SetData(const byte* data) override;
        virtual byte* GetData() const override;
        virtual const String& GetDebugName() const override;
        virtual TextureFormat GetFormat() const override;
        virtual u32 GetWidth() const override;
        virtual u32 GetHeight() const override;
        virtual u32 GetMipLevels() const override;
        virtual u8 GetUsageFlags() const override;
        virtual TextureFilter GetFilter() const override;
        virtual TextureWrap GetWrap() const override;
        
        inline wrl::ComPtr<ID3D12Resource2> GetD3DResource() const { return m_D3DResource; }

        inline const DX12DescriptorHandle& GetShaderResourceView() const 
        { 
            ATOM_ENGINE_ASSERT(m_Description.UsageFlags & TextureUsage::ShaderResource);
            return m_ShaderResourceView; 
        };

        inline const DX12DescriptorHandle& GetUnorderedAccessView(u32 mipLevel) const 
        { 
            ATOM_ENGINE_ASSERT((m_Description.UsageFlags & TextureUsage::UnorderedAccess) && mipLevel < m_Description.MipLevels);
            return m_UnorderedAccessViews[mipLevel];
        };

        inline DX12DescriptorHandle& GetRenderTargetView(u32 mipLevel) 
        { 
            ATOM_ENGINE_ASSERT((m_Description.UsageFlags & TextureUsage::RenderTarget) && mipLevel < m_Description.MipLevels);
            return m_RenderTargetViews[mipLevel]; 
        };

        inline const DX12DescriptorHandle& GetDepthStencilView(u32 mipLevel) const
        { 
            ATOM_ENGINE_ASSERT((m_Description.UsageFlags & TextureUsage::DepthBuffer) && mipLevel < m_Description.MipLevels);
            return m_DepthStencilViews[mipLevel];
        };

        inline const DX12DescriptorHandle& GetSampler() const { return m_Sampler; };
        static Ref<DX12Texture2D> CreateFromD3DResource(DX12Device& device, wrl::ComPtr<ID3D12Resource2> resource, TextureFilter filter, TextureWrap wrap);
    private:
        void CreateViewsAndSampler();
    private:
        wrl::ComPtr<ID3D12Resource2> m_D3DResource;
        TextureDescription           m_Description;
        DX12DescriptorHandle         m_ShaderResourceView;
        Vector<DX12DescriptorHandle> m_UnorderedAccessViews;
        Vector<DX12DescriptorHandle> m_RenderTargetViews;
        Vector<DX12DescriptorHandle> m_DepthStencilViews;
        DX12DescriptorHandle         m_Sampler;
        DX12Device&                  m_Device;
    };
}

#endif // ATOM_PLATFORM_WINDOWS