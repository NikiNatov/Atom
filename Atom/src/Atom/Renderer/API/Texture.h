#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Renderer.h"

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
        String DebugName = "";
        TextureFormat Format = TextureFormat::RGBA8;
        u32 Width = 0;
        u32 Height = 0;
        u32 MipLevels = 1;
        u8 UsageFlags = TextureUsage::ShaderResource;
        TextureFilter Filter = TextureFilter::Linear;
        TextureWrap Wrap = TextureWrap::Repeat;
    };

    class DX12Texture;

    class Texture
    {
    public:
        virtual ~Texture() = default;

        virtual void Release() = 0;
        virtual void DeferredRelease() = 0;
        virtual const String& GetDebugName() const = 0;
        virtual TextureType GetType() const = 0;
        virtual TextureFormat GetFormat() const = 0;
        virtual u32 GetWidth() const = 0;
        virtual u32 GetHeight() const = 0;
        virtual u32 GetMipLevels() const = 0;
        virtual u8 GetUsageFlags() const = 0;
        virtual TextureFilter GetFilter() const = 0;
        virtual TextureWrap GetWrap() const = 0;

        IMPL_API_CAST(Texture)

        static Ref<Texture> CreateTexture2D(const TextureDescription& description);
        static Ref<Texture> CreateTextureCube(const TextureDescription& description);
        static Ref<Texture> CreateSwapChainBuffer(u64 bufferHandle);
    };
}