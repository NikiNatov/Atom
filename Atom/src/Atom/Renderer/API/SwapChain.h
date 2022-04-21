#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Renderer.h"

namespace Atom
{
    class Texture;
    class TextureViewRT;
    class DX12SwapChain;

    class SwapChain
    {
    public:
        virtual ~SwapChain() = default;

        virtual void Present(bool vsync = true) = 0;
        virtual void Resize(u32 width, u32 height) = 0;
        virtual u32 GetWidth() const = 0;
        virtual u32 GetHeight() const = 0;
        virtual u32 GetCurrentBackBufferIndex() const = 0;
        virtual u32 GetBackBufferCount() const = 0;
        virtual const Texture* GetBackBuffer() const = 0;
        virtual const TextureViewRT* GetBackBufferRTV() const = 0;

        IMPL_API_CAST(SwapChain)

        static Ref<SwapChain> Create(u64 windowHandle, u32 width, u32 height);
    };
}