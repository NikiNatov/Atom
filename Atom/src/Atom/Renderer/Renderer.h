#pragma once

#include "Atom/Core/Core.h"

namespace Atom
{
    class Device;
    class CommandBuffer;
    class GraphicsPipeline;
    class Framebuffer;

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
        static void BeginFrame(const Ref<CommandBuffer>& commandBuffer);
        static void BeginRenderPass(const Ref<CommandBuffer>& commandBuffer, const Ref<Framebuffer>& framebuffer, bool clear = true);
        static void EndRenderPass(const Ref<CommandBuffer>& commandBuffer, const Ref<Framebuffer>& framebuffer);
        static void Draw(const Ref<CommandBuffer>& commandBuffer, const Ref<GraphicsPipeline>& pipeline, u32 count);
        static void EndFrame(const Ref<CommandBuffer>& commandBuffer);

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