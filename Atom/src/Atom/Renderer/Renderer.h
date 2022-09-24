#pragma once

#include "Atom/Core/Core.h"

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
    class Mesh;
    class DescriptorHeap;

    struct RendererConfig
    {
        u32 FramesInFlight = 3;
        u32 MaxDescriptorsPerHeap = 2048;
    };

    class Renderer
    {
    public:
        static void Initialize(const RendererConfig& config = RendererConfig());
        static void Shutdown();
        static void BeginFrame();
        static void BeginRenderPass(CommandBuffer* commandBuffer, const Framebuffer* framebuffer, bool clear = true);
        static void EndRenderPass(CommandBuffer* commandBuffer, const Framebuffer* framebuffer);
        static void RenderGeometry(CommandBuffer* commandBuffer, const GraphicsPipeline* pipeline, const Mesh* mesh, const ConstantBuffer* constantBuffer);
        static void EndFrame();

        static const RendererConfig& GetConfig();
        static u32 GetCurrentFrameIndex();
        static u32 GetFramesInFlight();
    private:
        inline static RendererConfig              ms_Config;
        inline static Vector<Ref<DescriptorHeap>> ms_ResourceHeaps;
        inline static Vector<Ref<DescriptorHeap>> ms_SamplerHeaps;
    };
}