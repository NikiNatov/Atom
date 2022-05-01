#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

namespace Atom
{
    enum class TextureType
    {
        None = 0,
        Texture2D,
        TextureCube,
        SwapChainBuffer
    };

    enum class TextureFormat
    {
        None = 0,
        RGBA8,
        RG16F, RGBA16F,
        RG32F, RGBA32F,
        Depth24Stencil8,
        Depth32
    };

    enum class TextureFilter
    {
        None = 0,
        Linear, 
        Nearest,
        Anisotropic
    };

    enum class TextureWrap
    {
        None = 0,
        Clamp, Repeat
    };

    enum TextureUsage : u8
    {
        ShaderResource = BIT(1),
        UnorderedAccess = BIT(2),
        RenderTarget = BIT(3),
        DepthBuffer = BIT(4)
    };

    struct TextureDescription
    {
        TextureFormat Format = TextureFormat::RGBA8;
        u32 Width = 0;
        u32 Height = 0;
        u32 MipLevels = 1;
        u8 UsageFlags = TextureUsage::ShaderResource;
        TextureFilter Filter = TextureFilter::Linear;
        TextureWrap Wrap = TextureWrap::Repeat;
    };

    class Texture
    {
    public:
        Texture(TextureType type, const TextureDescription& description, const char* debugName);
        Texture(TextureType type, ID3D12Resource* textureHandle, const char* debugName);
        ~Texture();

        TextureType GetType() const;
        TextureFormat GetFormat() const;
        u32 GetWidth() const;
        u32 GetHeight() const;
        u32 GetMipLevels() const;
        u8 GetUsageFlags() const;
        TextureFilter GetFilter() const;
        TextureWrap GetWrap() const;
        inline ComPtr<ID3D12Resource> GetD3DResource() const { return m_D3DResource; }
    private:
        ComPtr<ID3D12Resource> m_D3DResource;
        TextureType            m_Type;
        TextureDescription     m_Description;
    };
}