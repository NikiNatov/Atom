#include "atompch.h"
#include "CommandBuffer.h"

#include "Atom/Platform/DirectX12/DX12CommandBuffer.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<CommandBuffer> CommandBuffer::Create()
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateRef<DX12CommandBuffer>();
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }
    
}