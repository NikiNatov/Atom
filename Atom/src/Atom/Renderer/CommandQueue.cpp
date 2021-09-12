#include "atompch.h"
#include "CommandQueue.h"

#include "Platform/DirectX12/DX12CommandQueue.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Scope<CommandQueue> CommandQueue::CreateCommandQueue(const Device* device, const CommandQueueDesc& description)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DirectX12: return CreateScope<DX12CommandQueue>(device, description);
        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }
}
