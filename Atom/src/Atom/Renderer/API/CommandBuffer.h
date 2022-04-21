#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Renderer.h"

namespace Atom
{
    class Texture;
    class GraphicsPipeline;
    class Framebuffer;
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
        virtual void BeginRenderPass(const Ref<Framebuffer>& framebuffer, bool clear = false) = 0;
        virtual void EndRenderPass(const Ref<Framebuffer>& framebuffer) = 0;
        virtual void SetGraphicsPipeline(const Ref<GraphicsPipeline>& pipeline) = 0;
        virtual void Draw(u32 count) = 0;
        virtual void End() = 0;

        IMPL_API_CAST(CommandBuffer)

        static Ref<CommandBuffer> Create(const char* debugName = "Unnamed Command Buffer");
    };
}