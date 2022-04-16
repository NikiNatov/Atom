#pragma once

#include "Atom/Renderer/API/TextureView.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"
#include "DX12DescriptorAllocator.h"

namespace Atom
{
    class DX12TextureViewSR : public TextureViewSR
    {
    public:
        DX12TextureViewSR(const Ref<Texture>& texture);
        ~DX12TextureViewSR();

        virtual const Texture* GetTextureResource() const override;

        inline DX12DescriptorHandle& GetDescriptor() { return m_Descriptor; }
    private:
        Texture*                        m_Texture;
        D3D12_SHADER_RESOURCE_VIEW_DESC m_Description{};
        DX12DescriptorHandle            m_Descriptor;
    };

    class DX12TextureViewRT : public TextureViewRT
    {
    public:
        DX12TextureViewRT(const Ref<Texture>& texture, u32 mipLevel, u32 cubeFace);
        ~DX12TextureViewRT();

        virtual const Texture* GetTextureResource() const override;
        virtual u32 GetMipLevel() const override;
        virtual u32 GetCubeFace() const override;

        inline DX12DescriptorHandle& GetDescriptor() { return m_Descriptor; }
    private:
        Texture*                      m_Texture;
        D3D12_RENDER_TARGET_VIEW_DESC m_Description{};
        DX12DescriptorHandle          m_Descriptor;
        u32                           m_MipLevel;
        u32                           m_CubeFace;
    };

    class DX12TextureViewDS : public TextureViewDS
    {
    public:
        DX12TextureViewDS(const Ref<Texture>& texture, u32 mipLevel, u32 cubeFace);
        ~DX12TextureViewDS();

        virtual const Texture* GetTextureResource() const override;
        virtual u32 GetMipLevel() const override;
        virtual u32 GetCubeFace() const override;

        inline DX12DescriptorHandle& GetDescriptor() { return m_Descriptor; }
    private:
        Texture*                      m_Texture;
        D3D12_DEPTH_STENCIL_VIEW_DESC m_Description{};
        DX12DescriptorHandle          m_Descriptor;
        u32                           m_MipLevel;
        u32                           m_CubeFace;
    };

    class DX12TextureViewUA : public TextureViewUA
    {
    public:
        DX12TextureViewUA(const Ref<Texture>& texture, u32 mipLevel, u32 cubeFace);
        ~DX12TextureViewUA();

        virtual const Texture* GetTextureResource() const override;
        virtual u32 GetMipLevel() const override;
        virtual u32 GetCubeFace() const override;

        inline DX12DescriptorHandle& GetDescriptor() { return m_Descriptor; }
    private:
        Texture*                         m_Texture;
        D3D12_UNORDERED_ACCESS_VIEW_DESC m_Description{};
        DX12DescriptorHandle             m_Descriptor;
        u32                              m_MipLevel;
        u32                              m_CubeFace;
    };
}

#endif // ATOM_PLATFORM_WINDOWS