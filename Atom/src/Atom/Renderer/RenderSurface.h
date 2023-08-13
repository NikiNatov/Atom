#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Texture.h"

namespace Atom
{
    class RenderSurface
    {
    public:
        RenderSurface(const TextureDescription& description, const char* debugName);
        RenderSurface(const RenderSurface& aliasedSurface, u32 mipIndex, u32 sliceIndex);
        RenderSurface(ID3D12Resource* textureHandle, bool swapChainBuffer, const char* debugName);
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
        ResourceState GetInitialState() const;
        bool IsSwapChainBuffer() const;
        bool IsAlias() const;
        Ref<Texture> GetTexture() const;

        inline const TextureViewRT* GetRTV() const { return m_RTV.get(); }
        inline const TextureViewDS* GetDSV() const { return m_DSV.get(); }

    private:
        Ref<Texture>         m_Texture;
        Scope<TextureViewRT> m_RTV;
        Scope<TextureViewDS> m_DSV;
    };
}