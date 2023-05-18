#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Texture.h"

namespace Atom
{
    class RenderSurface
    {
    public:
        RenderSurface(const Ref<Texture>& texture, u32 mipIndex, u32 sliceIndex);
        ~RenderSurface();

        RenderSurface(const RenderSurface& rhs) = delete;
        RenderSurface& operator=(const RenderSurface& rhs) = delete;

        RenderSurface(RenderSurface&& rhs) noexcept;
        RenderSurface& operator=(RenderSurface&& rhs) noexcept;

        TextureFormat GetFormat() const;
        u32 GetWidth() const;
        u32 GetHeight() const;
        u32 GetDepth() const;
        u32 GetArraySize() const;
        TextureFlags GetFlags() const;
        const ClearValue& GetClearValue() const;
        const Ref<Texture> GetTexture() const;
        bool IsSwapChainBuffer() const;

        inline D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return m_RTVDescriptor; }
        inline D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const { return m_DSVDescriptor; }

    private:
        void CreateRTV(u32 mipIndex, u32 sliceIndex = UINT32_MAX);
        void CreateDSV(u32 mipIndex, u32 sliceIndex = UINT32_MAX);
    private:
        Ref<Texture> m_Texture;
        D3D12_CPU_DESCRIPTOR_HANDLE m_RTVDescriptor{ 0 };
        D3D12_CPU_DESCRIPTOR_HANDLE m_DSVDescriptor{ 0 };
    };
}