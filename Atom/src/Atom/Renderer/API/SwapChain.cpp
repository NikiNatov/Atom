#include "atompch.h"
#include "SwapChain.h"

#include "Atom/Platform/DirectX12/DX12SwapChain.h"

namespace Atom
{
    Ref<SwapChain> SwapChain::Create(Device& device, u64 windowHandle, u32 width, u32 height)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateRef<DX12SwapChain>(device, windowHandle, width, height);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }
}