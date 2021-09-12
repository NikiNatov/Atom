#pragma once

#include "Atom/Core/Core.h"

#include "Atom/Renderer/Adapter.h"
#include "Atom/Renderer/GraphicsCommandList.h"
#include "Atom/Renderer/CommandQueue.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"

namespace Atom { namespace Utils {


    static DXGI_GPU_PREFERENCE AtomAdapterPreferenceToDXGI(AdapterPreference preference)
    {
        switch (preference)
        {
            case AdapterPreference::None:               return DXGI_GPU_PREFERENCE_UNSPECIFIED;
            case AdapterPreference::MinimumPowered:     return DXGI_GPU_PREFERENCE_MINIMUM_POWER;
            case AdapterPreference::HighPerformance:    return DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
        }

        ATOM_ASSERT(false, "Unknown adapter preference!");
        return DXGI_GPU_PREFERENCE_UNSPECIFIED;
    }

    static D3D12_COMMAND_LIST_TYPE AtomCommandListTypeToD3D12(CommandListType commandListType)
    {
        switch (commandListType)
        {
            case CommandListType::Copy:     return D3D12_COMMAND_LIST_TYPE_COPY;
            case CommandListType::Compute:  return D3D12_COMMAND_LIST_TYPE_COMPUTE;
            case CommandListType::Direct:   return D3D12_COMMAND_LIST_TYPE_DIRECT;
        }

        ATOM_ASSERT(false, "Unknown command list type!");
        return D3D12_COMMAND_LIST_TYPE_DIRECT;
    }

    static D3D12_COMMAND_QUEUE_PRIORITY AtomCommandQueuePriorityToD3D12(CommandQueuePriority priority)
    {
        switch (priority)
        {
            case CommandQueuePriority::Normal: return D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
            case CommandQueuePriority::High:   return D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
        }

        ATOM_ASSERT(false, "Unknown command queue priority!");
        return D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    }
}}
#endif // ATOM_PLATFORM_WINDOWS