#pragma once

#include "Atom/Core/Core.h"

#include "Renderer.h"

namespace Atom
{
    class DX12CommandBuffer;

    class CommandBuffer
    {
    public:
        virtual ~CommandBuffer() = default;

        virtual void Begin() = 0;
        virtual void End() = 0;

        IMPL_API_CAST(CommandBuffer)

        static Ref<CommandBuffer> Create();
    };
}