#pragma once

#include "Atom/Core/Core.h"
#include "CommandQueue.h"
#include "Renderer.h"

namespace Atom
{
    class DX12Device;

    enum class GPUPreference
    {
        None = 0,
        MinimumPowered = 1,
        HighPerformance = 2
    };

    class Device
    {
    public:
        virtual ~Device() = default;

        virtual void Release() = 0;
        virtual bool IsRayTracingSupported() const = 0;
        virtual u64 GetSystemMemory() const = 0;
        virtual u64 GetVideoMemory() const = 0;
        virtual u64 GetSharedMemory() const = 0;
        virtual String GetDescription() const = 0;
        virtual CommandQueue& GetCommandQueue(CommandQueueType type) = 0;
        virtual void ProcessDeferredReleases() = 0;
        virtual void WaitIdle() const = 0;

        IMPL_API_CAST(Device)

        static Ref<Device> Create(GPUPreference gpuPreference);
    };
}