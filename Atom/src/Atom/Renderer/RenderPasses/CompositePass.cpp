#include "atompch.h"
#include "CompositePass.h"

#include "Atom/Core/Application.h"

#include "Atom/Renderer/Pipeline.h"
#include "Atom/Renderer/ShaderLibrary.h"
#include "Atom/Renderer/ResourceBarrier.h"
#include "Atom/Renderer/EngineResources.h"
#include "Atom/Renderer/RenderGraph/ResourceID.h"

#include <autogen/cpp/FullscreenQuadParams.h>

namespace Atom
{
    DEFINE_RID_RT(FinalOutput);
    DECLARE_RID_RT(SceneColorOutput);

    // -----------------------------------------------------------------------------------------------------------------------------
    CompositePass::CompositePass(RenderPassID passID, const String& name, u32 viewportWidth, u32 viewportHeight, bool renderToSwapChain)
        : RenderPass(passID, name, CommandQueueType::Graphics), m_ViewportWidth(viewportWidth), m_ViewportHeight(viewportHeight), m_RenderToSwapChain(renderToSwapChain)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CompositePass::Build(RenderPassBuilder& builder)
    {
        GraphicsPipelineDescription pipelineDesc;
        pipelineDesc.Topology = Topology::Triangles;
        pipelineDesc.Shader = ShaderLibrary::Get().Get<GraphicsShader>("CompositeShader");
        pipelineDesc.RenderTargetFormats = { TextureFormat::RGBA8 };
        pipelineDesc.Layout = {
            { "POSITION", ShaderDataType::Float3 },
            { "TEX_COORD", ShaderDataType::Float2 },
        };
        pipelineDesc.EnableBlend = false;
        pipelineDesc.EnableDepthTest = false;
        pipelineDesc.Wireframe = false;
        pipelineDesc.BackfaceCulling = true;

        builder.SetPipelineStateDesc(pipelineDesc);

        // Resources
        if (m_RenderToSwapChain)
        {
            builder.NewRT(RID(FinalOutput), Application::Get().GetWindow().GetSwapChain()->GetBackBuffer().get());
        }
        else
        {
            TextureDescription finalOutputDesc;
            finalOutputDesc.Format = TextureFormat::RGBA8;
            finalOutputDesc.Width = m_ViewportWidth;
            finalOutputDesc.Height = m_ViewportHeight;
            finalOutputDesc.ClearValue = ClearValue(0.2f, 0.2f, 0.2f, 1.0f);
            finalOutputDesc.Flags = TextureFlags::RenderTarget | TextureFlags::ShaderResource;
            finalOutputDesc.InitialState = ResourceState::RenderTarget;

            builder.NewRT(RID(FinalOutput), finalOutputDesc);
        }

        builder.Read(RID(SceneColorOutput));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CompositePass::Execute(RenderPassContext& context)
    {
        RenderSurface* finalOutput = context.GetRT(RID(FinalOutput))->GetData();
        Texture* sceneColorOutput = context.GetSR(RID(SceneColorOutput))->GetData();

        Ref<CommandBuffer> cmdBuffer = context.GetCommandBuffer();

        cmdBuffer->BeginRenderPass({ finalOutput }, true);

        SIG::FullscreenQuadParams params;
        params.SetTexture(sceneColorOutput);
        params.SetTextureSampler(EngineResources::LinearClampSampler.get());
        params.Compile();

        cmdBuffer->SetGraphicsDescriptorTables(ShaderBindPoint::Instance, params.GetResourceTable(), params.GetSamplerTable());
        cmdBuffer->SetVertexBuffer(EngineResources::QuadVertexBuffer.get());
        cmdBuffer->SetIndexBuffer(EngineResources::QuadIndexBuffer.get());
        cmdBuffer->DrawIndexed(EngineResources::QuadIndexBuffer->GetElementCount(), 1, 0, 0, 0);

        // TODO: Handle this case in the render graph
        if (m_RenderToSwapChain)
        {
            cmdBuffer->TransitionResource(Application::Get().GetWindow().GetSwapChain()->GetBackBuffer()->GetTexture().get(), ResourceState::Present);
        }
    }
}
