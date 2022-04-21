#include "atompch.h"
#include "SwapChain.h"

#include "Atom/Platform/DirectX12/DX12Texture.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture> Texture::CreateTexture2D(const TextureDescription& description, const char* debugName)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateRef<DX12Texture>(TextureType::Texture2D, description, debugName);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture> Texture::CreateTextureCube(const TextureDescription& description, const char* debugName)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateRef<DX12Texture>(TextureType::TextureCube, description, debugName);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture> Texture::CreateSwapChainBuffer(u64 bufferHandle, const char* debugName)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateRef<DX12Texture>(TextureType::SwapChainBuffer, bufferHandle, debugName);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }
}