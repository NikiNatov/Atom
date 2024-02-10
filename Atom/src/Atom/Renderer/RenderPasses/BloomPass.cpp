#include "atompch.h"
#include "BloomPass.h"

#include "Atom/Renderer/Pipeline.h"
#include "Atom/Renderer/ShaderLibrary.h"
#include "Atom/Renderer/EngineResources.h"
#include "Atom/Renderer/RenderGraph/ResourceID.h"

#include "autogen/cpp/BloomDownsampleParams.h"
#include "autogen/cpp/BloomUpsampleParams.h"
#include "autogen/cpp/BloomCompositeParams.h"

namespace Atom
{
    DECLARE_RID_RT(SceneColorOutput);
    DEFINE_RID_UA(BloomTexture);
    DEFINE_RID_RT(SceneBloom);

    // -----------------------------------------------------------------------------------------------------------------------------
    BloomDownsamplePass::BloomDownsamplePass(RenderPassID passID, const String& name, u32 viewportWidth, u32 viewportHeight, const BloomSettings& settings)
        : RenderPass(passID, name, CommandQueueType::Graphics), m_ViewportWidth(viewportWidth), m_ViewportHeight(viewportHeight), m_Settings(settings)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void BloomDownsamplePass::Build(RenderPassBuilder& builder)
    {
        ComputePipelineDescription pipelineDesc;
        pipelineDesc.Shader = ShaderLibrary::Get().Get<ComputeShader>("BloomDownsampleShader");

        builder.SetPipelineStateDesc(pipelineDesc);

        // Resources
        TextureDescription bloomTextureDesc;
        bloomTextureDesc.Format = TextureFormat::RGBA16F;
        bloomTextureDesc.Width = m_ViewportWidth;
        bloomTextureDesc.Height = m_ViewportHeight;
        bloomTextureDesc.MipLevels = std::min(Texture::CalculateMaxMipCount(m_ViewportWidth, m_ViewportHeight), (u32)m_Settings.DownSampleSteps);
        bloomTextureDesc.ClearValue = ClearValue(0.2f, 0.2f, 0.2f, 1.0f);
        bloomTextureDesc.Flags = TextureFlags::UnorderedAccess | TextureFlags::ShaderResource;
        bloomTextureDesc.InitialState = ResourceState::UnorderedAccess;

        builder.NewUA(RID(BloomTexture), bloomTextureDesc);

        builder.Read(RID(SceneColorOutput));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void BloomDownsamplePass::Execute(RenderPassContext& context)
    {
        Ref<CommandBuffer> cmdBuffer = context.GetCommandBuffer();

        u32 numMips = context.GetUA(RID(BloomTexture))->GetData()->GetMipLevels();
        for (u32 mip = 0; mip < numMips; mip++)
        {
            Texture* srcMip = mip == 0 ? context.GetSR(RID(SceneColorOutput))->GetData(0, 0) : context.GetUA(RID(BloomTexture))->GetData(mip - 1, 0);
            Texture* downsampledMip = context.GetUA(RID(BloomTexture))->GetData(mip, 0);

            SIG::BloomDownsampleParams params;
            params.SetSourceMipLevel(mip == 0 ? 0 : mip - 1);
            params.SetDownsampledMipLevel(mip);
            params.SetSourceTexture(srcMip);
            params.SetDownsampledTexture(downsampledMip);
            params.Compile();

            cmdBuffer->SetComputeSIG(params);

            u32 threadGroupsX = ((glm::max(downsampledMip->GetWidth() / 8, 1u) + 2 - 1) / 2) * 2;
            u32 threadGroupsY = ((glm::max(downsampledMip->GetHeight() / 8, 1u) + 2 - 1) / 2) * 2;
            cmdBuffer->Dispatch(downsampledMip->GetWidth(), downsampledMip->GetHeight(), 1);

            cmdBuffer->AddUAVBarrier(context.GetUA(RID(BloomTexture))->GetData());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    BloomUpsamplePass::BloomUpsamplePass(RenderPassID passID, const String& name, u32 viewportWidth, u32 viewportHeight, const BloomSettings& settings)
        : RenderPass(passID, name, CommandQueueType::Graphics), m_ViewportWidth(viewportWidth), m_ViewportHeight(viewportHeight), m_Settings(settings)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void BloomUpsamplePass::Build(RenderPassBuilder& builder)
    {
        ComputePipelineDescription pipelineDesc;
        pipelineDesc.Shader = ShaderLibrary::Get().Get<ComputeShader>("BloomUpsampleShader");

        builder.SetPipelineStateDesc(pipelineDesc);

        // Resources
        builder.Write(RID(BloomTexture));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void BloomUpsamplePass::Execute(RenderPassContext& context)
    {
        Ref<CommandBuffer> cmdBuffer = context.GetCommandBuffer();

        u32 numMips = context.GetUA(RID(BloomTexture))->GetData()->GetMipLevels();
        for (s32 mip = numMips - 1; mip >= 1; mip--)
        {
            Texture* srcMip = context.GetUA(RID(BloomTexture))->GetData(mip, 0);
            Texture* upsampledMip = context.GetUA(RID(BloomTexture))->GetData(mip - 1, 0);

            SIG::BloomUpsampleParams params;
            params.SetSourceMipLevel(mip);
            params.SetFilterRadius(m_Settings.FilterRadius);
            params.SetSourceTexture(srcMip);
            params.SetUpsampledTexture(upsampledMip);
            params.Compile();

            cmdBuffer->SetComputeSIG(params);

            u32 threadGroupsX = ((glm::max(upsampledMip->GetWidth() / 8, 1u) + 2 - 1) / 2) * 2;
            u32 threadGroupsY = ((glm::max(upsampledMip->GetHeight() / 8, 1u) + 2 - 1) / 2) * 2;
            cmdBuffer->Dispatch(upsampledMip->GetWidth(), upsampledMip->GetHeight(), 1);

            cmdBuffer->AddUAVBarrier(context.GetUA(RID(BloomTexture))->GetData());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    BloomCompositePass::BloomCompositePass(RenderPassID passID, const String& name, u32 viewportWidth, u32 viewportHeight, const BloomSettings& settings)
        : RenderPass(passID, name, CommandQueueType::Graphics), m_ViewportWidth(viewportWidth), m_ViewportHeight(viewportHeight), m_Settings(settings)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void BloomCompositePass::Build(RenderPassBuilder& builder)
    {
        GraphicsPipelineDescription pipelineDesc;
        pipelineDesc.Topology = Topology::Triangles;
        pipelineDesc.Shader = ShaderLibrary::Get().Get<GraphicsShader>("BloomCompositeShader");
        pipelineDesc.RenderTargetFormats = { TextureFormat::RGBA16F };
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
        TextureDescription sceneBloomDesc;
        sceneBloomDesc.Format = TextureFormat::RGBA16F;
        sceneBloomDesc.Width = m_ViewportWidth;
        sceneBloomDesc.Height = m_ViewportHeight;
        sceneBloomDesc.ClearValue = ClearValue(0.2f, 0.2f, 0.2f, 1.0f);
        sceneBloomDesc.Flags = TextureFlags::RenderTarget | TextureFlags::ShaderResource;
        sceneBloomDesc.InitialState = ResourceState::RenderTarget;

        builder.NewRT(RID(SceneBloom), sceneBloomDesc);

        builder.Read(RID(BloomTexture));
        builder.Read(RID(SceneColorOutput));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void BloomCompositePass::Execute(RenderPassContext& context)
    {
        RenderSurface* sceneBloom = context.GetRT(RID(SceneBloom))->GetData();
        Texture* sceneColorOutput = context.GetSR(RID(SceneColorOutput))->GetData();
        Texture* bloomTexture = context.GetSR(RID(BloomTexture))->GetData();

        Ref<CommandBuffer> cmdBuffer = context.GetCommandBuffer();

        cmdBuffer->BeginRenderPass({ sceneBloom }, true);

        SIG::BloomCompositeParams params;
        params.SetBloomStrength(m_Settings.BloomStrength);
        params.SetBloomTexture(bloomTexture);
        params.SetSceneTexture(sceneColorOutput);
        params.Compile();

        cmdBuffer->SetGraphicsSIG(params);
        cmdBuffer->SetVertexBuffer(EngineResources::QuadVertexBuffer.get());
        cmdBuffer->SetIndexBuffer(EngineResources::QuadIndexBuffer.get());
        cmdBuffer->DrawIndexed(EngineResources::QuadIndexBuffer->GetElementCount(), 1, 0, 0, 0);
    }
}
