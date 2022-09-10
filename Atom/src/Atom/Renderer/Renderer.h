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
    class Material;

    struct RendererConfig
    {
        u32 FramesInFlight = 3;
        u32 MaxDescriptorsPerHeap = 2048;
    };

    class Renderer
    {
    public:
        static void Initialize(const RendererConfig& config = RendererConfig());
        static void BeginFrame();
        static void BeginRenderPass(CommandBuffer* commandBuffer, const Framebuffer* framebuffer, bool clear = true);
        static void EndRenderPass(CommandBuffer* commandBuffer, const Framebuffer* framebuffer);
        static void RenderGeometry(CommandBuffer* commandBuffer, const GraphicsPipeline* pipeline, const VertexBuffer* vertexBuffer, const IndexBuffer* indexBuffer, const ConstantBuffer* constantBuffer, const Material* material);
        static void EndFrame();

        static const RendererConfig& GetConfig();
        static u32 GetCurrentFrameIndex();
        static u32 GetFramesInFlight();
    private:
        static RendererConfig ms_Config;
        static Vector<Ref<DescriptorHeap>> ms_ResourceHeaps;
        static Vector<Ref<DescriptorHeap>> ms_SamplerHeaps;
    };
}