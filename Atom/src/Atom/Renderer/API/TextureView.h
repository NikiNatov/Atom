#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Renderer.h"

namespace Atom
{
    class Texture;
    class DX12TextureViewSR;
    class DX12TextureViewRT;
    class DX12TextureViewDS;
    class DX12TextureViewUA;

    class TextureViewSR
    {
    public:
        virtual ~TextureViewSR() = default;

        virtual const Texture* GetTextureResource() const = 0;

        IMPL_API_CAST(TextureViewSR)

        static Ref<TextureViewSR> Create(const Ref<Texture>& texture);
    };

    class TextureViewRT
    {
    public:
        virtual ~TextureViewRT() = default;

        virtual const Texture* GetTextureResource() const = 0;
        virtual u32 GetMipLevel() const = 0;
        virtual u32 GetCubeFace() const = 0;

        IMPL_API_CAST(TextureViewRT)

        static Ref<TextureViewRT> Create(const Ref<Texture>& texture, u32 mipLevel = 0, u32 cubeFace = UINT32_MAX);
    };

    class TextureViewDS
    {
    public:
        virtual ~TextureViewDS() = default;

        virtual const Texture* GetTextureResource() const = 0;
        virtual u32 GetMipLevel() const = 0;
        virtual u32 GetCubeFace() const = 0;

        IMPL_API_CAST(TextureViewDS)

        static Ref<TextureViewDS> Create(const Ref<Texture>& texture, u32 mipLevel = 0, u32 cubeFace = UINT32_MAX);
    };

    class TextureViewUA
    {
    public:
        virtual ~TextureViewUA() = default;

        virtual const Texture* GetTextureResource() const = 0;
        virtual u32 GetMipLevel() const = 0;
        virtual u32 GetCubeFace() const = 0;

        IMPL_API_CAST(TextureViewUA)

        static Ref<TextureViewUA> Create(const Ref<Texture>& texture, u32 mipLevel = 0, u32 cubeFace = UINT32_MAX);
    };
}