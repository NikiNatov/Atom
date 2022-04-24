#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Renderer.h"

namespace Atom
{
    class Texture;
    class GraphicsPipeline;
    class Framebuffer;
    class VertexBuffer;
    class IndexBuffer;
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
        virtual void BeginRenderPass(const Framebuffer* framebuffer, bool clear = false) = 0;
        virtual void EndRenderPass(const Framebuffer* framebuffer) = 0;
        virtual void SetGraphicsPipeline(const GraphicsPipeline* pipeline) = 0;
        virtual void SetVertexBuffer(const VertexBuffer* vertexBuffer) = 0;
        virtual void SetIndexBuffer(const IndexBuffer* indexBuffer) = 0;
        virtual void DrawIndexed(u32 indexCount) = 0;
        virtual void End() = 0;

        IMPL_API_CAST(CommandBuffer)

        static Ref<CommandBuffer> Create(const char* debugName = "Unnamed Command Buffer");
    };
}