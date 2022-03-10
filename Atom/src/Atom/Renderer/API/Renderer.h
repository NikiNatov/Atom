#pragma once

#include "Atom/Core/Core.h"

namespace Atom
{
    class Device;
    class CommandQueue;
    class CommandBuffer;

    enum class RenderAPI
    {
        None = 0,
        DirectX12
    };

    class Renderer
    {
    public:
        Renderer() = default;

        static void Initialize();
        static void BeginFrame(const Ref<CommandBuffer>& commandBuffer);
        static void EndFrame(const Ref<CommandBuffer>& commandBuffer);

        static u32 GetCurrentFrameIndex();
        static Device& GetDevice();
        static u32 GetFramesInFlight() { return FRAMES_IN_FLIGHT; }

        static void SetAPI(RenderAPI api) { ms_RenderAPI = api; }
        static RenderAPI GetAPI() { return ms_RenderAPI; }
    private:
        static RenderAPI          ms_RenderAPI;
        static constexpr u32      FRAMES_IN_FLIGHT = 3;
    };
}