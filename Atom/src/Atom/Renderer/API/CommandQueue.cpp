#include "atompch.h"
#include "CommandQueue.h"

#include "Atom/Platform/DirectX12/DX12CommandQueue.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Scope<CommandQueue> CommandQueue::Create(CommandQueueType type)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateScope<DX12CommandQueue>(type);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }
}