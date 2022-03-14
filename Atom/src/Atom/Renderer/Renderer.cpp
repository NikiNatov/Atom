#include "atompch.h"
#include "Renderer.h"

#include "Atom/Core/Application.h"
#include "Atom/Renderer/API/Device.h"
#include "Atom/Renderer/API/CommandBuffer.h"
#include "Atom/Renderer/API/Texture.h"
#include "Atom/Renderer/API/TextureView.h"

namespace Atom
{
    RenderAPI Renderer::ms_RenderAPI = RenderAPI::None;
    Ref<Device> Renderer::ms_Device = nullptr;

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::Initialize()
    {
        ms_Device = Device::Create(GPUPreference::HighPerformance);
        ms_Device->Initialize();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::BeginFrame(const Ref<CommandBuffer>& commandBuffer)
    {
        auto backBuffer = Application::Get().GetWindow().GetSwapChain().GetBackBuffer();

        commandBuffer->Begin();
        commandBuffer->TransitionResource(backBuffer, ResourceState::Present, ResourceState::RenderTarget);

        const f32 color[] = { 0.1f, 0.1f, 0.1f, 1.0f };
        commandBuffer->ClearRenderTarget(TextureViewRT::Create(backBuffer), color);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::EndFrame(const Ref<CommandBuffer>& commandBuffer)
    {
        commandBuffer->TransitionResource(Application::Get().GetWindow().GetSwapChain().GetBackBuffer(), ResourceState::RenderTarget, ResourceState::Present);
        commandBuffer->End();

        ms_Device->GetCommandQueue(CommandQueueType::Graphics).ExecuteCommandList(commandBuffer);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::SetAPI(RenderAPI api)
    {
        ms_RenderAPI = api;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderAPI Renderer::GetAPI()
    {
        return ms_RenderAPI;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Device& Renderer::GetDevice()
    {
        return *ms_Device;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Renderer::GetCurrentFrameIndex()
    {
        return Application::Get().GetWindow().GetSwapChain().GetCurrentBackBufferIndex();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Renderer::GetFramesInFlight()
    {
        return FRAMES_IN_FLIGHT;
    }
}