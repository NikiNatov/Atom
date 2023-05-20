#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"
#include "Atom/Renderer/TextureView.h"

#include <glm/glm.hpp>

namespace Atom
{
    enum class TextureType
    {
        None = 0,
        Texture2D,
        Texture3D,
    };

    enum class TextureFormat
    {
        None = 0,
        R8,
        RGBA8,
        RG16F, RGBA16F,
        RG32F, RGBA32F,
        Depth24Stencil8,
        Depth32
    };

    struct DepthStencilValue
    {
        f32 DepthValue;
        u8 StencilValue;

        DepthStencilValue(f32 depth = 1.0f, u8 stencil = 0)
            : DepthValue(depth), StencilValue(stencil) {}
    };

    struct ClearValue
    {
        union
        {
            glm::vec4 Color;
            DepthStencilValue DepthStencil = {};
        };

        ClearValue()
            : Color(0.0f, 0.0f, 0.0f, 1.0f), DepthStencil() {}

        ClearValue(f32 r, f32 g, f32 b, f32 a)
            : Color(r, g, b, a) {}

        ClearValue(f32 depthValue, u8 stencilValue)
            : DepthStencil(depthValue, stencilValue) {}
    };

    enum class TextureFlags : u8
    {
        None            = 0,
        ShaderResource  = BIT(0),
        UnorderedAccess = BIT(1),
        RenderTarget    = BIT(2),
        DepthStencil    = BIT(3),
        SwapChainBuffer = BIT(4),
        CubeMap         = BIT(5),
        
        DefaultFlags = ShaderResource
    };

    IMPL_ENUM_OPERATORS(TextureFlags);

    struct TextureDescription
    {
        TextureType Type = TextureType::Texture2D;
        TextureFormat Format = TextureFormat::RGBA8;
        u32 Width = 1;
        u32 Height = 1;
        u32 Depth = 1;
        u32 ArraySize = 1;
        u32 MipLevels = 1;
        TextureFlags Flags = TextureFlags::DefaultFlags;
        ClearValue ClearValue = {};
    };

    class Texture
    {
    public:
        Texture(const TextureDescription& description, const char* debugName);
        Texture(const Texture& aliasedTexture, u32 mipIndex, u32 sliceIndex, const char* debugName);
        Texture(ID3D12Resource* textureHandle, TextureFlags additionalFlags, const char* debugName);
        ~Texture();
        
        Texture(const Texture& rhs) = delete;
        Texture& operator=(const Texture& rhs) = delete;

        Texture(Texture&& rhs) noexcept;
        Texture& operator=(Texture&& rhs) noexcept;

        TextureType GetType() const;
        TextureFormat GetFormat() const;
        u32 GetWidth() const;
        u32 GetHeight() const;
        u32 GetDepth() const;
        u32 GetArraySize() const;
        u32 GetMipLevels() const;
        TextureFlags GetFlags() const;
        const ClearValue& GetClearValue() const;
        const TextureDescription& GetDescription() const;

        inline ComPtr<ID3D12Resource> GetD3DResource() const { return m_D3DResource; }
        inline const TextureViewRO* GetSRV() const { return m_SRV.get(); }
        inline const TextureViewRW* GetUAV() const { return m_UAV.get(); }

    public:
        static u32 CalculateMaxMipCount(u32 width, u32 height);
        static u32 CalculateSubresource(u32 mip, u32 slice, u32 mipCount, u32 arraySize);
    private:
        ComPtr<ID3D12Resource> m_D3DResource;
        TextureDescription     m_Description;
        Scope<TextureViewRO>   m_SRV;
        Scope<TextureViewRW>   m_UAV;
        bool                   m_IsAlias = false;
    };
}