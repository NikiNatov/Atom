#include "atompch.h"
#include "GeometryPass.h"

#include "Atom/Renderer/Pipeline.h"
#include "Atom/Renderer/Renderer.h"
#include "Atom/Renderer/RenderGraph/ResourceID.h"
#include "Atom/Scene/SceneRenderer.h"

namespace Atom
{
    DECLARE_RID_RT(SceneColorOutput);
    DECLARE_RID_DS(SceneDepthBuffer);

    // -----------------------------------------------------------------------------------------------------------------------------
    GeometryPass::GeometryPass(RenderPassID id, const String& name)
        : RenderPass(id, name, CommandQueueType::Graphics)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void GeometryPass::Build(RenderPassBuilder& builder)
    {
        // Pipelines
        {
            GraphicsPipelineDescription pipelineDesc;
            pipelineDesc.Topology = Topology::Triangles;
            pipelineDesc.Shader = Renderer::GetShaderLibrary().Get<GraphicsShader>("MeshPBRShader");
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

            Renderer::GetPipelineLibrary().Load<GraphicsPipeline>("GeometryPass_StaticMesh", pipelineDesc);
        }

        {
            GraphicsPipelineDescription pipelineDesc;
            pipelineDesc.Topology = Topology::Triangles;
            pipelineDesc.Shader = Renderer::GetShaderLibrary().Get<GraphicsShader>("MeshPBRAnimatedShader");
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

            Renderer::GetPipelineLibrary().Load<GraphicsPipeline>("GeometryPass_AnimatedMesh", pipelineDesc);
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
        GraphicsPipeline* staticMeshPipeline = Renderer::GetPipelineLibrary().Get<GraphicsPipeline>("GeometryPass_StaticMesh").get();
        GraphicsPipeline* animatedMeshPipeline = Renderer::GetPipelineLibrary().Get<GraphicsPipeline>("GeometryPass_AnimatedMesh").get();

        struct MeshPBRInstanceConstants
        {
            glm::mat4 Transform;
            u32 BoneTransformOffset;
        };

        Ref<CommandBuffer> cmdBuffer = context.GetCommandBuffer();

        cmdBuffer->BeginRenderPass({ sceneColorOutput, depthBuffer });

        // Render animated meshes
        cmdBuffer->SetGraphicsPipeline(animatedMeshPipeline);
        cmdBuffer->SetGraphicsConstants(ShaderBindPoint::Frame, context.GetFrameConstantBuffer().get());
        cmdBuffer->SetGraphicsDescriptorTables(ShaderBindPoint::Frame, context.GetFrameResourceTable(), context.GetFrameSamplerTable());

        const auto& animatedMeshes = context.GetSceneData().AnimatedMeshes;
        for (u32 i = 0; i < context.GetSceneData().AnimatedMeshes.size(); i++)
        {
            MeshPBRInstanceConstants constants = { animatedMeshes[i].Transform, animatedMeshes[i].BoneTransformIndex };
            cmdBuffer->SetGraphicsConstants(ShaderBindPoint::Instance, &constants, sizeof(MeshPBRInstanceConstants) / 4);
            Renderer::RenderMesh(cmdBuffer, animatedMeshes[i].Mesh, animatedMeshes[i].SubmeshIndex, animatedMeshes[i].Material);
        }

        // Render static meshes
        cmdBuffer->SetGraphicsPipeline(staticMeshPipeline);
        cmdBuffer->SetGraphicsConstants(ShaderBindPoint::Frame, context.GetFrameConstantBuffer().get());
        cmdBuffer->SetGraphicsDescriptorTables(ShaderBindPoint::Frame, context.GetFrameResourceTable(), context.GetFrameSamplerTable());

        const auto& staticMeshes = context.GetSceneData().StaticMeshes;
        for (u32 i = 0; i < staticMeshes.size(); i++)
        {
            MeshPBRInstanceConstants constants = { staticMeshes[i].Transform, UINT32_MAX };
            cmdBuffer->SetGraphicsConstants(ShaderBindPoint::Instance, &constants, sizeof(MeshPBRInstanceConstants) / 4);
            Renderer::RenderMesh(cmdBuffer, staticMeshes[i].Mesh, staticMeshes[i].SubmeshIndex, staticMeshes[i].Material);
        }
    }
}
