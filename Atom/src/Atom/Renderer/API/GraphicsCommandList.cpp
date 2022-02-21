#include "atompch.h"
#include "GraphicsCommandList.h"

#include "Atom/Platform/DirectX12/DX12GraphicsCommandList.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Scope<CommandAllocator> CommandAllocator::CreateCommandAllocator(const Device* device, CommandListType commandListType)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DirectX12: return CreateScope<DX12CommandAllocator>(device, commandListType);
        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Scope<GraphicsCommandList> GraphicsCommandList::CreateGraphicsCommandList(const Device* device, CommandListType type, const CommandAllocator* allocator)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DirectX12: return CreateScope<DX12GraphicsCommandList>(device, type, allocator);
        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }
    
}