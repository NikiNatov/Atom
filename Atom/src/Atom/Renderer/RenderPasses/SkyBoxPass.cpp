#include "atompch.h"
#include "SkyBoxPass.h"

#include "Atom/Renderer/Pipeline.h"
#include "Atom/Renderer/Renderer.h"
#include "Atom/Renderer/RenderGraph/ResourceID.h"

namespace Atom
{
    DEFINE_RID_RT(SceneColorOutput);
    DEFINE_RID_DS(SceneDepthBuffer);

    // -----------------------------------------------------------------------------------------------------------------------------
    SkyBoxPass::SkyBoxPass(RenderPassID passID, const String& name, u32 viewportWidth, u32 viewportHeight)
        : RenderPass(passID, name, CommandQueueType::Graphics), m_ViewportWidth(viewportWidth), m_ViewportHeight(viewportHeight)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SkyBoxPass::Build(RenderPassBuilder& builder)
    {
        // Pipelines
        {
            GraphicsPipelineDescription pipelineDesc;
            pipelineDesc.Topology = Topology::Triangles;
            pipelineDesc.Shader = Renderer::GetShaderLibrary().Get<GraphicsShader>("SkyBoxShader");
            pipelineDesc.RenderTargetFormats = { TextureFormat::RGBA16F, TextureFormat::Depth24Stencil8 };
            pipelineDesc.Layout = {
                { "POSITION", ShaderDataType::Float3 },
                { "TEX_COORD", ShaderDataType::Float2 },
            };
            pipelineDesc.EnableBlend = false;
            pipelineDesc.EnableDepthTest = false;
            pipelineDesc.Wireframe = false;
            pipelineDesc.BackfaceCulling = true;

            Renderer::GetPipelineLibrary().Load<GraphicsPipeline>("SkyBoxPass_Default", pipelineDesc);
        }

        // Resources
        TextureDescription colorOutputDesc;
        colorOutputDesc.Format = TextureFormat::RGBA16F;
        colorOutputDesc.Width = m_ViewportWidth;
        colorOutputDesc.Height = m_ViewportHeight;
        colorOutputDesc.ClearValue = ClearValue(0.2f, 0.2f, 0.2f, 1.0f);
        colorOutputDesc.Flags = TextureFlags::RenderTarget | TextureFlags::ShaderResource;
        colorOutputDesc.InitialState = ResourceState::RenderTarget;

        builder.NewRT(RID(SceneColorOutput), colorOutputDesc);

        TextureDescription depthBufferDesc;
        depthBufferDesc.Format = TextureFormat::Depth24Stencil8;
        depthBufferDesc.Width = m_ViewportWidth;
        depthBufferDesc.Height = m_ViewportHeight;
        depthBufferDesc.ClearValue = ClearValue(1.0f, 0);
        depthBufferDesc.Flags = TextureFlags::DepthStencil | TextureFlags::ShaderResource;
        depthBufferDesc.InitialState = ResourceState::DepthWrite;

        builder.NewDS(RID(SceneDepthBuffer), depthBufferDesc);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SkyBoxPass::Execute(RenderPassContext& context)
    {
        RenderSurface* sceneColorOutput = context.GetRT(RID(SceneColorOutput))->GetData();
        RenderSurface* depthBuffer = context.GetDS_RW(RID(SceneDepthBuffer))->GetData();
        GraphicsPipeline* pipeline = Renderer::GetPipelineLibrary().Get<GraphicsPipeline>("SkyBoxPass_Default").get();

        Ref<CommandBuffer> cmdBuffer = context.GetCommandBuffer();

        cmdBuffer->BeginRenderPass({ sceneColorOutput, depthBuffer }, true);
        cmdBuffer->SetGraphicsPipeline(pipeline);
        cmdBuffer->SetGraphicsConstants(ShaderBindPoint::Frame, context.GetFrameConstantBuffer().get());
        cmdBuffer->SetGraphicsDescriptorTables(ShaderBindPoint::Frame, context.GetFrameResourceTable(), context.GetFrameSamplerTable());

        Renderer::RenderFullscreenQuad(cmdBuffer, nullptr);
    }
}
