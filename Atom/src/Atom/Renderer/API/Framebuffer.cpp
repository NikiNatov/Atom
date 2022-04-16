#include "atompch.h"
#include "Framebuffer.h"

#include "Atom/Platform/DirectX12/DX12Framebuffer.h"

namespace Atom
{
    Ref<Framebuffer> Framebuffer::Create(const FramebufferDescription& description)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateRef<DX12Framebuffer>(description);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }
}
