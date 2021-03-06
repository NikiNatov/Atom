#include "atompch.h"
#include "Renderer.h"

#include "Atom/Core/Application.h"
#include "Atom/Renderer/Device.h"
#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/CommandQueue.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/GraphicsPipeline.h"
#include "Atom/Renderer/Framebuffer.h"
#include "Atom/Renderer/Buffer.h"

namespace Atom
{
    RendererConfig Renderer::ms_Config;

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::Initialize(const RendererConfig& config)
    {
        ms_Config = config;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::BeginFrame(CommandBuffer* commandBuffer)
    {
        Device::Get().ProcessDeferredReleases(GetCurrentFrameIndex());

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
    void Renderer::RenderGeometry(CommandBuffer* commandBuffer, const GraphicsPipeline* pipeline, const VertexBuffer* vertexBuffer, const IndexBuffer* indexBuffer, const ConstantBuffer* constantBuffer)
    {
        commandBuffer->SetGraphicsPipeline(pipeline);
        commandBuffer->SetVertexBuffer(vertexBuffer);
        commandBuffer->SetIndexBuffer(indexBuffer);
        // TODO: Find a better way of setting constant buffers
        commandBuffer->SetConstantBuffer(0, constantBuffer);
        commandBuffer->DrawIndexed(indexBuffer->GetElementCount());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::EndFrame(CommandBuffer* commandBuffer)
    {
        commandBuffer->End();
        auto fence = Device::Get().GetCommandQueue(CommandQueueType::Graphics)->ExecuteCommandList(commandBuffer);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const RendererConfig& Renderer::GetConfig()
    {
        return ms_Config;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Renderer::GetCurrentFrameIndex()
    {
        return Application::Get().GetWindow().GetSwapChain()->GetCurrentBackBufferIndex();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Renderer::GetFramesInFlight()
    {
        return ms_Config.FramesInFlight;
    }
}