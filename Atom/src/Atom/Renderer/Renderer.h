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

    enum class RenderAPI
    {
        None = 0,
        DirectX12
    };

    struct RendererConfig
    {
        u32 FramesInFlight = 3;
        RenderAPI API = RenderAPI::None;
    };

    class Renderer
    {
    public:
        static void Initialize();
        static void BeginFrame(CommandBuffer* commandBuffer);
        static void BeginRenderPass(CommandBuffer* commandBuffer, const Framebuffer* framebuffer, bool clear = true);
        static void EndRenderPass(CommandBuffer* commandBuffer, const Framebuffer* framebuffer);
        static void RenderGeometry(CommandBuffer* commandBuffer, const GraphicsPipeline* pipeline, const VertexBuffer* vertexBuffer, const IndexBuffer* indexBuffer);
        static void EndFrame(CommandBuffer* commandBuffer);

        static void SetAPI(RenderAPI api);
        static RenderAPI GetAPI();
        static Device& GetDevice();
        static u32 GetCurrentFrameIndex();
        static u32 GetFramesInFlight();
    private:
        static Ref<Device>   ms_Device;
        static RenderAPI     ms_RenderAPI;
        static constexpr u32 FRAMES_IN_FLIGHT = 3;
    };
}