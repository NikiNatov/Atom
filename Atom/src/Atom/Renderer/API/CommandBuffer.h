#pragma once

#include "Atom/Core/Core.h"

#include "Renderer.h"

namespace Atom
{
    class DX12CommandBuffer;
    class Texture2D;

    enum class ResourceState
    {
        Common = 0,
        Present,
        RenderTarget,
        UnorderedAccess,
        PixelShaderResource
    };

    class CommandBuffer
    {
    public:
        virtual ~CommandBuffer() = default;

        virtual void Begin() = 0;
        virtual void TransitionResource(const Ref<Texture2D>& texture, ResourceState beforeState, ResourceState afterState) = 0;
        virtual void End() = 0;

        IMPL_API_CAST(CommandBuffer)

        static Ref<CommandBuffer> Create();
    };
}