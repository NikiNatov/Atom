#include "atompch.h"
#include "TextureView.h"

#include "Atom/Platform/DirectX12/DX12TextureView.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<TextureViewSR> TextureViewSR::Create(const Ref<Texture>& resource)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateRef<DX12TextureViewSR>(resource);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<TextureViewRT> TextureViewRT::Create(const Ref<Texture>& resource, u32 mipLevel, u32 cubeFace)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateRef<DX12TextureViewRT>(resource, mipLevel, cubeFace);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<TextureViewDS> TextureViewDS::Create(const Ref<Texture>& resource, u32 mipLevel, u32 cubeFace)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateRef<DX12TextureViewDS>(resource, mipLevel, cubeFace);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<TextureViewUA> TextureViewUA::Create(const Ref<Texture>& resource, u32 mipLevel, u32 cubeFace)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateRef<DX12TextureViewUA>(resource, mipLevel, cubeFace);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }
}