#include "atompch.h"
#include "GeometryPass.h"

#include "Atom/Renderer/Pipeline.h"
#include "Atom/Renderer/ShaderLibrary.h"
#include "Atom/Renderer/RenderGraph/ResourceID.h"

#include <autogen/cpp/MeshDrawParams.h>

namespace Atom
{
    DECLARE_RID_RT(SceneColorOutput);
    DECLARE_RID_DS(SceneDepthBuffer);

    // -----------------------------------------------------------------------------------------------------------------------------
    GeometryPass::GeometryPass(RenderPassID id, const String& name, const Vector<MeshEntry>& meshEntries, bool isAnimated)
        : RenderPass(id, name, CommandQueueType::Graphics), m_MeshEntries(meshEntries), m_IsAnimated(isAnimated)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void GeometryPass::Build(RenderPassBuilder& builder)
    {
        if(!m_IsAnimated)
        {
            GraphicsPipelineDescription pipelineDesc;
            pipelineDesc.Topology = Topology::Triangles;
            pipelineDesc.Shader = ShaderLibrary::Get().Get<GraphicsShader>("MeshPBRShader");
            pipelineDesc.RenderTargetFormats = { TextureFormat::RGBA16F, TextureFormat::Depth24Stencil8 };
            pipelineDesc.Layout = {
                { "POSITION", ShaderDataType::Float3 },
                { "TEX_COORD", ShaderDataType::Float2 },
                { "NORMAL", ShaderDataType::Float3 },
                { "TANGENT", ShaderDataType::Float3 },
                { "BITANGENT", ShaderDataType::Float3 },
            };
            pipelineDesc.EnableBlend = true;
            pipelineDesc.EnableDepthTest = true;
            pipelineDesc.Wireframe = false;
            pipelineDesc.BackfaceCulling = true;

            builder.SetPipelineStateDesc(pipelineDesc);
        }
        else
        {
            GraphicsPipelineDescription pipelineDesc;
            pipelineDesc.Topology = Topology::Triangles;
            pipelineDesc.Shader = ShaderLibrary::Get().Get<GraphicsShader>("MeshPBRAnimatedShader");
            pipelineDesc.RenderTargetFormats = { TextureFormat::RGBA16F, TextureFormat::Depth24Stencil8 };
            pipelineDesc.Layout = {
                { "POSITION", ShaderDataType::Float3 },
                { "TEX_COORD", ShaderDataType::Float2 },
                { "NORMAL", ShaderDataType::Float3 },
                { "TANGENT", ShaderDataType::Float3 },
                { "BITANGENT", ShaderDataType::Float3 },
                { "BONE_IDS", ShaderDataType::Uint4 },
                { "BONE_WEIGHTS", ShaderDataType::Float4 },
            };
            pipelineDesc.EnableBlend = true;
            pipelineDesc.EnableDepthTest = true;
            pipelineDesc.Wireframe = false;
            pipelineDesc.BackfaceCulling = true;

            builder.SetPipelineStateDesc(pipelineDesc);
        }

        // Resources
        builder.Write(RID(SceneColorOutput));
        builder.Write(RID(SceneDepthBuffer));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void GeometryPass::Execute(RenderPassContext& context)
    {
        RenderSurface* sceneColorOutput = context.GetRT(RID(SceneColorOutput))->GetData();
        RenderSurface* depthBuffer = context.GetDS_RW(RID(SceneDepthBuffer))->GetData();

        Ref<CommandBuffer> cmdBuffer = context.GetCommandBuffer();

        cmdBuffer->BeginRenderPass({ sceneColorOutput, depthBuffer });

        for (const MeshEntry& meshEntry : m_MeshEntries)
        {
            const Submesh& submesh = meshEntry.Mesh->GetSubmeshes()[meshEntry.SubmeshIndex];
            Ref<Material> material = meshEntry.Material ? meshEntry.Material : meshEntry.Mesh->GetMaterialTable()->GetMaterial(submesh.MaterialIndex);

            // Transition material textures
            for (auto& [_, texture] : material->GetTextures())
                if (texture)
                    cmdBuffer->TransitionResource(texture->GetResource().get(), ResourceState::PixelShaderRead);

            material->UpdateForRendering();

            SIG::MeshDrawParams meshDrawParams;
            meshDrawParams.SetTransform(meshEntry.Transform);
            meshDrawParams.SetBoneTransformOffset(meshEntry.BoneTransformOffset);
            meshDrawParams.Compile();

            cmdBuffer->SetGraphicsSIG(meshDrawParams);
            cmdBuffer->SetGraphicsSIG(*material->GetSIG());
            cmdBuffer->SetVertexBuffer(meshEntry.Mesh->GetVertexBuffer().get());
            cmdBuffer->SetIndexBuffer(meshEntry.Mesh->GetIndexBuffer().get());
            cmdBuffer->DrawIndexed(submesh.IndexCount, 1, submesh.StartIndex, submesh.StartVertex, 0);
        }
    }
}
