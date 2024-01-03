#include "atompch.h"
#include "ShadowPass.h"

#include "Atom/Renderer/Pipeline.h"
#include "Atom/Renderer/ShaderLibrary.h"
#include "Atom/Renderer/RenderGraph/ResourceID.h"

#include <autogen/cpp/MeshDrawParams.h>
#include <autogen/cpp/ShadowDepthPassParams.h>

namespace Atom
{
    DEFINE_RID_DS(CascadeShadowMap);

    // -----------------------------------------------------------------------------------------------------------------------------
    ShadowPass::ShadowPass(RenderPassID id, const String& name, const Vector<ShadowCascade>& shadowCascades, const Vector<MeshEntry>& meshEntries, bool isAnimated)
        : RenderPass(id, name, CommandQueueType::Graphics), m_ShadowCascades(shadowCascades), m_MeshEntries(meshEntries), m_IsAnimated(isAnimated)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ShadowPass::Build(RenderPassBuilder& builder)
    {
        if (!m_IsAnimated)
        {
            GraphicsPipelineDescription pipelineDesc;
            pipelineDesc.Topology = Topology::Triangles;
            pipelineDesc.Shader = ShaderLibrary::Get().Get<GraphicsShader>("ShadowDepthPassShader");
            pipelineDesc.RenderTargetFormats = { TextureFormat::Depth32 };
            pipelineDesc.Layout = {
                { "POSITION", ShaderDataType::Float3 },
                { "TEX_COORD", ShaderDataType::Float2 },
                { "NORMAL", ShaderDataType::Float3 },
                { "TANGENT", ShaderDataType::Float3 },
                { "BITANGENT", ShaderDataType::Float3 },
            };
            pipelineDesc.EnableBlend = false;
            pipelineDesc.EnableDepthTest = true;
            pipelineDesc.Wireframe = false;
            pipelineDesc.BackfaceCulling = true;

            builder.SetPipelineStateDesc(pipelineDesc);

            // Resources
            TextureDescription shadowMapDesc;
            shadowMapDesc.Type = TextureType::Texture2D;
            shadowMapDesc.Format = TextureFormat::Depth32;
            shadowMapDesc.ArraySize = m_ShadowCascades.size();
            shadowMapDesc.Width = 4096;
            shadowMapDesc.Height = 4096;
            shadowMapDesc.Flags = TextureFlags::ShaderResource | TextureFlags::DepthStencil;
            shadowMapDesc.ClearValue = ClearValue(1.0f, 0);
            shadowMapDesc.InitialState = ResourceState::DepthWrite;

            builder.NewDS(RID(CascadeShadowMap), shadowMapDesc);
        }
        else
        {
            GraphicsPipelineDescription pipelineDesc;
            pipelineDesc.Topology = Topology::Triangles;
            pipelineDesc.Shader = ShaderLibrary::Get().Get<GraphicsShader>("ShadowDepthPassAnimatedShader");
            pipelineDesc.RenderTargetFormats = { TextureFormat::Depth32 };
            pipelineDesc.Layout = {
                { "POSITION", ShaderDataType::Float3 },
                { "TEX_COORD", ShaderDataType::Float2 },
                { "NORMAL", ShaderDataType::Float3 },
                { "TANGENT", ShaderDataType::Float3 },
                { "BITANGENT", ShaderDataType::Float3 },
                { "BONE_IDS", ShaderDataType::Uint4 },
                { "BONE_WEIGHTS", ShaderDataType::Float4 },
            };
            pipelineDesc.EnableBlend = false;
            pipelineDesc.EnableDepthTest = true;
            pipelineDesc.Wireframe = false;
            pipelineDesc.BackfaceCulling = true;

            builder.SetPipelineStateDesc(pipelineDesc);

            builder.Write(RID(CascadeShadowMap));
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ShadowPass::Execute(RenderPassContext& context)
    {
        Ref<CommandBuffer> cmdBuffer = context.GetCommandBuffer();

        for (uint32_t cascadeIndex = 0; cascadeIndex < m_ShadowCascades.size(); cascadeIndex++)
        {
            RenderSurface* shadowMap = context.GetDS_RW(RID(CascadeShadowMap))->GetData(0, cascadeIndex);
            cmdBuffer->BeginRenderPass({ shadowMap }, !m_IsAnimated);

            SIG::ShadowDepthPassParams shadowParams;
            shadowParams.SetCascadeIndex(cascadeIndex);
            shadowParams.Compile();

            cmdBuffer->SetGraphicsSIG(shadowParams);

            for (const MeshEntry& meshEntry : m_MeshEntries)
            {
                const Submesh& submesh = meshEntry.Mesh->GetSubmeshes()[meshEntry.SubmeshIndex];

                cmdBuffer->SetGraphicsSIG(meshEntry.DrawParams);
                cmdBuffer->SetVertexBuffer(meshEntry.Mesh->GetVertexBuffer().get());
                cmdBuffer->SetIndexBuffer(meshEntry.Mesh->GetIndexBuffer().get());
                cmdBuffer->DrawIndexed(submesh.IndexCount, 1, submesh.StartIndex, submesh.StartVertex, 0);
            }
        }
    }
}
