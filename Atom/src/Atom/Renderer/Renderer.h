#pragma once

#include "Atom/Core/Core.h"

#include "DescriptorHeap.h"

namespace Atom
{
    class Device;
    class CommandBuffer;
    class GraphicsPipeline;
    class Framebuffer;
    class VertexBuffer;
    class IndexBuffer;
    class ConstantBuffer;

    struct RendererConfig
    {
        u32 FramesInFlight = 3;
        u32 MaxDescriptorsPerHeap = 100'000;
    };

    class Renderer
    {
    public:
        static void Initialize(const RendererConfig& config = RendererConfig());
        static void BeginFrame(CommandBuffer* commandBuffer);
        static void BeginRenderPass(CommandBuffer* commandBuffer, const Framebuffer* framebuffer, bool clear = true);
        static void EndRenderPass(CommandBuffer* commandBuffer, const Framebuffer* framebuffer);
        static void RenderGeometry(CommandBuffer* commandBuffer, const GraphicsPipeline* pipeline, const VertexBuffer* vertexBuffer, const IndexBuffer* indexBuffer, const ConstantBuffer* constantBuffer);
        static void EndFrame(CommandBuffer* commandBuffer);

        static const RendererConfig& GetConfig();
        static u32 GetCurrentFrameIndex();
        static u32 GetFramesInFlight();
    private:
        static RendererConfig ms_Config;
    };
}