#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

namespace Atom
{
    class Texture;

    class TextureViewSR
    {
    public:
        TextureViewSR(const Ref<Texture>& texture);
        ~TextureViewSR();

        const Texture* GetTextureResource() const;
        inline D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptor() const { return m_Descriptor; }
    private:
        Texture* m_Texture;
        D3D12_SHADER_RESOURCE_VIEW_DESC m_Description{};
        D3D12_CPU_DESCRIPTOR_HANDLE     m_Descriptor;
    };

    class TextureViewRT
    {
    public:
        TextureViewRT(const Ref<Texture>& texture, u32 mipLevel = 0, u32 cubeFace = UINT32_MAX);
        ~TextureViewRT();

        const Texture* GetTextureResource() const;
        u32 GetMipLevel() const;
        u32 GetCubeFace() const;

        inline D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptor() const { return m_Descriptor; }
    private:
        Texture* m_Texture;
        D3D12_RENDER_TARGET_VIEW_DESC m_Description{};
        D3D12_CPU_DESCRIPTOR_HANDLE   m_Descriptor;
        u32                           m_MipLevel;
        u32                           m_CubeFace;
    };

    class TextureViewDS
    {
    public:
        TextureViewDS(const Ref<Texture>& texture, u32 mipLevel = 0, u32 cubeFace = UINT32_MAX);
        ~TextureViewDS();

        const Texture* GetTextureResource() const;
        u32 GetMipLevel() const;
        u32 GetCubeFace() const;

        inline D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptor() const { return m_Descriptor; }
    private:
        Texture* m_Texture;
        D3D12_DEPTH_STENCIL_VIEW_DESC m_Description{};
        D3D12_CPU_DESCRIPTOR_HANDLE   m_Descriptor;
        u32                           m_MipLevel;
        u32                           m_CubeFace;
    };

    class TextureViewUA
    {
    public:
        TextureViewUA(const Ref<Texture>& texture, u32 mipLevel = 0, u32 cubeFace = UINT32_MAX);
        ~TextureViewUA();

        const Texture* GetTextureResource() const;
        u32 GetMipLevel() const;
        u32 GetCubeFace() const;

        inline D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptor() const { return m_Descriptor; }
    private:
        Texture* m_Texture;
        D3D12_UNORDERED_ACCESS_VIEW_DESC m_Description{};
        D3D12_CPU_DESCRIPTOR_HANDLE      m_Descriptor;
        u32                              m_MipLevel;
        u32                              m_CubeFace;
    };
}