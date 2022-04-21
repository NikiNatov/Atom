#include "atompch.h"
#include "CommandQueue.h"

#include "Atom/Platform/DirectX12/DX12CommandQueue.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Scope<CommandQueue> CommandQueue::Create(CommandQueueType type, const char* debugName)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateScope<DX12CommandQueue>(type, debugName);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }
}