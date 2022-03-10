#pragma once

#include "Atom/Core/Core.h"
#include "Renderer.h"

namespace Atom
{
    class CommandBuffer;
    class DX12CommandQueue;

    enum class CommandQueueType
    {
        Graphics = 0,
        Compute = 1,
        Copy = 2,
    };

    class CommandQueue
    {
    public:
        virtual ~CommandQueue() = default;

        virtual u64 Signal() = 0;
        virtual void WaitForFenceValue(u64 value) = 0;
        virtual void Flush() = 0;
        virtual u64 ExecuteCommandList(const Ref<CommandBuffer>& commandBuffer) = 0;
        virtual u64 ExecuteCommandLists(const Vector<Ref<CommandBuffer>>& commandBuffers) = 0;
        virtual CommandQueueType GetQueueType() const = 0;

        IMPL_API_CAST(CommandQueue)

        static Scope<CommandQueue> Create(Device& device, CommandQueueType type);
    };

}