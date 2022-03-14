#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Renderer.h"

namespace Atom
{
    class Texture;
    class TextureViewRT;
    class DX12CommandBuffer;

    enum class ResourceState
    {
        Common,
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
        virtual void TransitionResource(const Ref<Texture>& texture, ResourceState beforeState, ResourceState afterState) = 0;
        virtual void ClearRenderTarget(const Ref<TextureViewRT>& renderTarget, const f32* color) = 0;
        virtual void End() = 0;

        IMPL_API_CAST(CommandBuffer)

        static Ref<CommandBuffer> Create();
    };
}