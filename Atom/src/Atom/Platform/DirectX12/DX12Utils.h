#pragma once

#include "Atom/Core/Core.h"


#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"

#include "Atom/Renderer/API/Device.h"
#include "Atom/Renderer/API/CommandQueue.h"

namespace Atom { namespace Utils {


    static DXGI_GPU_PREFERENCE AtomGPUPreferenceToDXGI(GPUPreference preference)
    {
        switch (preference)
        {
            case GPUPreference::None:            return DXGI_GPU_PREFERENCE_UNSPECIFIED;
            case GPUPreference::MinimumPowered:  return DXGI_GPU_PREFERENCE_MINIMUM_POWER;
            case GPUPreference::HighPerformance: return DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
        }

        ATOM_ASSERT(false, "Unknown GPU preference!");
        return DXGI_GPU_PREFERENCE_UNSPECIFIED;
    }

    static D3D12_COMMAND_LIST_TYPE AtomCommandQueueTypeToD3D12(CommandQueueType type)
    {
        switch (type)
        {
            case CommandQueueType::Graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
            case CommandQueueType::Compute:  return D3D12_COMMAND_LIST_TYPE_COMPUTE;
            case CommandQueueType::Copy:     return D3D12_COMMAND_LIST_TYPE_COPY;
        }

        ATOM_ASSERT(false, "Unknown queue type!");
        return D3D12_COMMAND_LIST_TYPE_DIRECT;
    }
}}
#endif // ATOM_PLATFORM_WINDOWS