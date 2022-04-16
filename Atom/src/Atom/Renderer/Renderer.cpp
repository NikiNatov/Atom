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
    Ref<GraphicsPipeline> Renderer::ms_DefaultPipeline = nullptr;

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::Initialize()
    {
        ms_Device = Device::Create(GPUPreference::HighPerformance);
        ms_Device->Initialize();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::CreatePipelines()
    {
        FramebufferDescription fbDesc;
        fbDesc.SwapChainFrameBuffer = true;
        fbDesc.ClearColor[0] = 0.2f;
        fbDesc.ClearColor[1] = 0.2f;
        fbDesc.ClearColor[2] = 0.2f;
        fbDesc.ClearColor[3] = 1.0f;
        fbDesc.Attachments[AttachmentPoint::Color0] = { TextureFormat::RGBA8, TextureFilter::Linear, TextureWrap::Clamp };
        fbDesc.Attachments[AttachmentPoint::DepthStencil] = { TextureFormat::Depth24Stencil8, TextureFilter::Linear, TextureWrap::Clamp };

        GraphicsPipelineDescription pipelineDesc;
        pipelineDesc.Topology = Topology::Triangles;
        pipelineDesc.Shader = Shader::Create("assets/shaders/Shader.hlsl");
        pipelineDesc.Framebuffer = Framebuffer::Create(fbDesc);
        pipelineDesc.Layout = {
            { "POSITION", ShaderDataType::Float3 },
            { "NORMAL", ShaderDataType::Float3 },
            { "TEXCOORD", ShaderDataType::Float2 }
        };
        pipelineDesc.EnableDepthTest = false;
        pipelineDesc.EnableBlend = true;
        pipelineDesc.Wireframe = false;
        pipelineDesc.BackfaceCulling = false;

        ms_DefaultPipeline = GraphicsPipeline::Create(pipelineDesc);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::BeginFrame(const Ref<CommandBuffer>& commandBuffer)
    {
        commandBuffer->Begin();
        commandBuffer->TransitionResource(ms_DefaultPipeline->GetFramebuffer()->GetAttachmnt(AttachmentPoint::Color0), ResourceState::Present, ResourceState::RenderTarget);
        commandBuffer->BeginRenderPass(ms_DefaultPipeline->GetFramebuffer(), true);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::EndFrame(const Ref<CommandBuffer>& commandBuffer)
    {
        commandBuffer->SetGraphicsPipeline(ms_DefaultPipeline);
        commandBuffer->Draw(4);

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