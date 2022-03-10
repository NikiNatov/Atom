#include "atompch.h"
#include "SwapChain.h"

#include "Atom/Platform/DirectX12/DX12Texture.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture2D> Texture2D::Create(const TextureDescription& description)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateRef<DX12Texture2D>(description);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture2D> Texture2D::CreateFromFile(const String& filename, const TextureDescription& description)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateRef<DX12Texture2D>(filename, description);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }
}