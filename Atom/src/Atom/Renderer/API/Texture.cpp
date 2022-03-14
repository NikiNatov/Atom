#include "atompch.h"
#include "SwapChain.h"

#include "Atom/Platform/DirectX12/DX12Texture.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture> Texture::CreateTexture2D(const TextureDescription& description)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateRef<DX12Texture>(TextureType::Texture2D, description);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture> Texture::CreateTextureCube(const TextureDescription& description)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateRef<DX12Texture>(TextureType::TextureCube, description);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture> Texture::CreateSwapChainBuffer(u64 bufferHandle)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateRef<DX12Texture>(TextureType::SwapChainBuffer, bufferHandle);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }
}