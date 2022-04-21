#include "atompch.h"
#include "Renderer.h"

#include "Atom/Core/Application.h"
#include "Atom/Renderer/API/Device.h"
#include "Atom/Renderer/API/CommandBuffer.h"
#include "Atom/Renderer/API/Texture.h"
#include "Atom/Renderer/API/TextureView.h"
#include "Atom/Renderer/API/GraphicsPipeline.h"
#include "Atom/Renderer/API/Framebuffer.h"

namespace Atom
{
    RenderAPI Renderer::ms_RenderAPI = RenderAPI::None;
    Ref<Device> Renderer::ms_Device = nullptr;

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::Initialize()
    {
        ms_Device = Device::Create(GPUPreference::HighPerformance, "Main Device");
        ms_Device->Initialize();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::BeginFrame(CommandBuffer* commandBuffer)
    {
        commandBuffer->Begin();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::BeginRenderPass(CommandBuffer* commandBuffer, const Framebuffer* framebuffer, bool clear)
    {
        commandBuffer->BeginRenderPass(framebuffer, clear);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::EndRenderPass(CommandBuffer* commandBuffer, const Framebuffer* framebuffer)
    {
        commandBuffer->EndRenderPass(framebuffer);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::Draw(CommandBuffer* commandBuffer, const GraphicsPipeline* pipeline, u32 count)
    {
        commandBuffer->SetGraphicsPipeline(pipeline);
        commandBuffer->Draw(count);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::EndFrame(CommandBuffer* commandBuffer)
    {
        commandBuffer->End();
        auto fence = ms_Device->GetCommandQueue(CommandQueueType::Graphics).ExecuteCommandList(commandBuffer);
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