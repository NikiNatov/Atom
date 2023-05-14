#include "atompch.h"
#include "SceneRenderer.h"

#include "Atom/Renderer/Device.h"
#include "Atom/Renderer/Renderer.h"
#include "Atom/Renderer/Material.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    SceneRenderer::~SceneRenderer()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::Initialize()
    {
        const PipelineLibrary& pipelineLib = Renderer::GetPipelineLibrary();
        const ShaderLibrary& shaderLib = Renderer::GetShaderLibrary();

        // Set pipelines
        m_GeometryPipeline = pipelineLib.Get<GraphicsPipeline>("MeshPBRPipeline");
        m_AnimatedGeometryPipeline = pipelineLib.Get<GraphicsPipeline>("MeshPBRAnimatedPipeline");
        m_CompositePipeline = pipelineLib.Get<GraphicsPipeline>("CompositePipeline");
        m_SkyBoxPipeline = pipelineLib.Get<GraphicsPipeline>("SkyBoxPipeline");
        m_FullScreenQuadPipeline = pipelineLib.Get<GraphicsPipeline>("FullscreenQuadPipeline");

        u32 framesInFlight = Renderer::GetFramesInFlight();

        // Create frame constant buffers
        m_FrameConstants.resize(framesInFlight);
        m_FrameCBs.resize(framesInFlight);

        BufferDescription frameCBDesc;
        frameCBDesc.ElementCount = 1;
        frameCBDesc.ElementSize = sizeof(FrameConstants);
        frameCBDesc.IsDynamic = true;

        for(u32 frameIdx = 0 ; frameIdx < framesInFlight; frameIdx++)
            m_FrameCBs[frameIdx] = CreateRef<ConstantBuffer>(frameCBDesc, fmt::format("FrameCB[{}]", frameIdx).c_str());

        // Create lights structured buffer
        m_Lights.resize(framesInFlight);
        m_LightsSBs.resize(framesInFlight);

        BufferDescription lightsSBDesc;
        lightsSBDesc.ElementCount = 1;
        lightsSBDesc.ElementSize = sizeof(Light);
        lightsSBDesc.IsDynamic = true;

        for (u32 i = 0; i < framesInFlight; i++)
            m_LightsSBs[i] = CreateRef<StructuredBuffer>(lightsSBDesc, fmt::format("LightsSB[{}]", i).c_str());

        // Create bone transforms structured buffers
        m_BoneTransforms.resize(framesInFlight);
        m_BoneTransformsSBs.resize(framesInFlight);

        BufferDescription animSBDesc;
        animSBDesc.ElementCount = 1;
        animSBDesc.ElementSize = sizeof(glm::mat4);
        animSBDesc.IsDynamic = true;

        for (u32 i = 0; i < framesInFlight; i++)
            m_BoneTransformsSBs[i] = CreateRef<StructuredBuffer>(animSBDesc, fmt::format("BoneTransformsSB[{}]", i).c_str());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::BeginScene(Camera& camera, const glm::mat4& cameraTransform, const Ref<LightEnvironment>& lightEnvironment)
    {
        u32 currentFrameIdx = Renderer::GetCurrentFrameIndex();

        // Set frame constants
        m_FrameConstants[currentFrameIdx].ViewMatrix = glm::inverse(cameraTransform);
        m_FrameConstants[currentFrameIdx].ProjectionMatrix = camera.GetProjection();
        m_FrameConstants[currentFrameIdx].InvViewProjMatrix = glm::inverse(m_FrameConstants[currentFrameIdx].ProjectionMatrix * m_FrameConstants[currentFrameIdx].ViewMatrix);
        m_FrameConstants[currentFrameIdx].CameraPosition = cameraTransform[3];
        m_FrameConstants[currentFrameIdx].CameraExposure = 0.5f; // Hard-coded for now
        m_FrameConstants[currentFrameIdx].NumLights = lightEnvironment->GetLights().size();

        // Set lights data
        m_Lights[currentFrameIdx] = lightEnvironment;

        // Reset bone transforms
        m_BoneTransforms[currentFrameIdx].clear();

        // Reset draw lists
        m_DrawList.clear();
        m_DrawListAnimated.clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::BeginScene(EditorCamera& editorCamera, const Ref<LightEnvironment>& lightEnvironment)
    {
        u32 currentFrameIdx = Renderer::GetCurrentFrameIndex();

        // Set camera transforms
        m_FrameConstants[currentFrameIdx].ViewMatrix = editorCamera.GetViewMatrix();
        m_FrameConstants[currentFrameIdx].ProjectionMatrix = editorCamera.GetProjection();
        m_FrameConstants[currentFrameIdx].InvViewProjMatrix = glm::inverse(m_FrameConstants[currentFrameIdx].ProjectionMatrix * m_FrameConstants[currentFrameIdx].ViewMatrix);
        m_FrameConstants[currentFrameIdx].CameraPosition = editorCamera.GetPosition();
        m_FrameConstants[currentFrameIdx].CameraExposure = 0.5f; // Hard-coded for now
        m_FrameConstants[currentFrameIdx].NumLights = lightEnvironment->GetLights().size();

        // Set lights data
        m_Lights[currentFrameIdx] = lightEnvironment;

        // Reset bone transforms
        m_BoneTransforms[currentFrameIdx].clear();

        // Reset draw lists
        m_DrawList.clear();
        m_DrawListAnimated.clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<MaterialTable>& materialTable)
    {
        if (!mesh)
            return;

        const auto& submeshes = mesh->GetSubmeshes();
        for (u32 submeshIdx = 0 ; submeshIdx < submeshes.size(); submeshIdx++)
        {
            const Submesh& submesh = submeshes[submeshIdx];
            const Ref<MaterialTable>& meshMaterialTable = mesh->GetMaterialTable();
            Ref<Material> material = materialTable && materialTable->HasMaterial(submesh.MaterialIndex) ? materialTable->GetMaterial(submesh.MaterialIndex) : meshMaterialTable->GetMaterial(submesh.MaterialIndex);

            DrawCommand& drawCommand = m_DrawList.emplace_back();
            drawCommand.Mesh = mesh;
            drawCommand.SubmeshIndex = submeshIdx;
            drawCommand.Transform = transform;
            drawCommand.Material = material ? material : Renderer::GetErrorMaterial();
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::SubmitAnimatedMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<MaterialTable>& materialTable, const Ref<Skeleton>& skeleton)
    {
        if (!mesh || !skeleton)
            return;
        
        u32 currentFrameIdx = Renderer::GetCurrentFrameIndex();

        // Submit draw command for each submesh
        const auto& submeshes = mesh->GetSubmeshes();
        for (u32 submeshIdx = 0; submeshIdx < submeshes.size(); submeshIdx++)
        {
            const Submesh& submesh = submeshes[submeshIdx];
            const Ref<MaterialTable>& meshMaterialTable = mesh->GetMaterialTable();
            Ref<Material> material = materialTable && materialTable->HasMaterial(submesh.MaterialIndex) ? materialTable->GetMaterial(submesh.MaterialIndex) : meshMaterialTable->GetMaterial(submesh.MaterialIndex);

            AnimatedDrawCommand& drawCommand = m_DrawListAnimated.emplace_back();
            drawCommand.Mesh = mesh;
            drawCommand.SubmeshIndex = submeshIdx;
            drawCommand.Transform = transform;
            drawCommand.Material = material ? material : Renderer::GetErrorMaterialAnimated();
            drawCommand.BoneTransformIndex = m_BoneTransforms[currentFrameIdx].size();
        }

        // Set all bone transforms
        for (auto& bone : skeleton->GetBones())
            m_BoneTransforms[currentFrameIdx].push_back(bone.AnimatedTransform);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::OnViewportResize(u32 width, u32 height)
    {
        m_GeometryPipeline->GetFramebuffer()->Resize(width, height);
        m_CompositePipeline->GetFramebuffer()->Resize(width, height);
        m_FullScreenQuadPipeline->GetFramebuffer()->Resize(width, height);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::PreRender(Ref<CommandBuffer> commandBuffer)
    {
        u32 currentFrameIdx = Renderer::GetCurrentFrameIndex();

        // Update frame constant buffer data
        void* data = m_FrameCBs[currentFrameIdx]->Map(0, 0);
        memcpy(data, &m_FrameConstants[currentFrameIdx], sizeof(FrameConstants));
        m_FrameCBs[currentFrameIdx]->Unmap();

        // Update lights structured buffer data
        const auto& lights = m_Lights[currentFrameIdx]->GetLights();
        if (!lights.empty())
        {
            if (m_LightsSBs[currentFrameIdx]->GetElementCount() != lights.size())
            {
                BufferDescription sbDesc;
                sbDesc.ElementCount = lights.size();
                sbDesc.ElementSize = sizeof(Light);
                sbDesc.IsDynamic = true;

                m_LightsSBs[currentFrameIdx] = CreateRef<StructuredBuffer>(sbDesc, fmt::format("LightsSB[{}]", currentFrameIdx).c_str());
            }

            void* lightsData = m_LightsSBs[currentFrameIdx]->Map(0, 0);
            memcpy(lightsData, lights.data(), sizeof(Light) * lights.size());
            m_LightsSBs[currentFrameIdx]->Unmap();
        }

        // Update bone transforms structured buffer data
        if (!m_BoneTransforms[currentFrameIdx].empty())
        {
            if (m_BoneTransformsSBs[currentFrameIdx]->GetElementCount() != m_BoneTransforms[currentFrameIdx].size())
            {
                BufferDescription animSBDesc;
                animSBDesc.ElementCount = m_BoneTransforms[currentFrameIdx].size();
                animSBDesc.ElementSize = sizeof(glm::mat4);
                animSBDesc.IsDynamic = true;

                m_BoneTransformsSBs[currentFrameIdx] = CreateRef<StructuredBuffer>(animSBDesc, fmt::format("BoneTransformsSB[{}]", currentFrameIdx).c_str());
            }

            void* boneTransformData = m_BoneTransformsSBs[currentFrameIdx]->Map(0, 0);
            memcpy(boneTransformData, m_BoneTransforms[currentFrameIdx].data(), sizeof(glm::mat4) * m_BoneTransforms[currentFrameIdx].size());
            m_BoneTransformsSBs[currentFrameIdx]->Unmap();
        }
        
        // Create descriptor tables for the frame
        auto& device = Device::Get();

        D3D12_CPU_DESCRIPTOR_HANDLE frameResourceDescriptors[] = { 
            m_Lights[currentFrameIdx]->GetEnvironmentMap()->GetSRV(),
            m_Lights[currentFrameIdx]->GetIrradianceMap()->GetSRV(),
            Renderer::GetBRDF()->GetSRV(),
            m_LightsSBs[currentFrameIdx]->GetSRV(),
            m_BoneTransformsSBs[currentFrameIdx]->GetSRV()
        };

        D3D12_CPU_DESCRIPTOR_HANDLE frameSamplerDescriptors[] = {
            m_Lights[currentFrameIdx]->GetEnvironmentMap()->GetSampler(),
            m_Lights[currentFrameIdx]->GetIrradianceMap()->GetSampler(),
            Renderer::GetBRDF()->GetSampler()
        };

        m_FrameResourceTable = device.GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocateTransient(_countof(frameResourceDescriptors));
        device.CopyDescriptors(m_FrameResourceTable, _countof(frameResourceDescriptors), frameResourceDescriptors, DescriptorHeapType::ShaderResource);

        m_FrameSamplerTable = device.GetGPUDescriptorHeap(DescriptorHeapType::Sampler)->AllocateTransient(_countof(frameSamplerDescriptors));
        device.CopyDescriptors(m_FrameSamplerTable, _countof(frameSamplerDescriptors), frameSamplerDescriptors, DescriptorHeapType::Sampler);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::Flush()
    {
        CommandQueue* gfxQueue = Device::Get().GetCommandQueue(CommandQueueType::Graphics);
        Ref<CommandBuffer> gfxCmdBuffer = gfxQueue->GetCommandBuffer();
        gfxCmdBuffer->Begin();
        gfxCmdBuffer->SetDescriptorHeaps(Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource), Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler));

        PreRender(gfxCmdBuffer);
        GeometryPass(gfxCmdBuffer);
        CompositePass(gfxCmdBuffer);

        gfxCmdBuffer->End();
        gfxQueue->ExecuteCommandList(gfxCmdBuffer);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::GeometryPass(Ref<CommandBuffer> commandBuffer)
    {
        u32 currentFrameIdx = Renderer::GetCurrentFrameIndex();

        Renderer::BeginRenderPass(commandBuffer, m_GeometryPipeline->GetFramebuffer());

        // Render skybox
        commandBuffer->SetGraphicsPipeline(m_SkyBoxPipeline.get());
        commandBuffer->SetGraphicsConstants(ShaderBindPoint::Frame, m_FrameCBs[currentFrameIdx].get());
        commandBuffer->SetGraphicsDescriptorTables(ShaderBindPoint::Frame, m_FrameResourceTable.GetBaseGpuDescriptor(), m_FrameSamplerTable.GetBaseGpuDescriptor());
        Renderer::RenderFullscreenQuad(commandBuffer, nullptr);

        struct MeshPBRInstanceConstants
        {
            glm::mat4 Transform;
            u32 BoneTransformOffset;
        };

        // Render animated meshes
        commandBuffer->SetGraphicsPipeline(m_AnimatedGeometryPipeline.get());
        commandBuffer->SetGraphicsConstants(ShaderBindPoint::Frame, m_FrameCBs[currentFrameIdx].get());
        commandBuffer->SetGraphicsDescriptorTables(ShaderBindPoint::Frame, m_FrameResourceTable.GetBaseGpuDescriptor(), m_FrameSamplerTable.GetBaseGpuDescriptor());

        for (u32 i = 0; i < m_DrawListAnimated.size(); i++)
        {
            MeshPBRInstanceConstants constants = { m_DrawListAnimated[i].Transform, m_DrawListAnimated[i].BoneTransformIndex };
            commandBuffer->SetGraphicsConstants(ShaderBindPoint::Instance, &constants, sizeof(MeshPBRInstanceConstants) / 4);
            Renderer::RenderMesh(commandBuffer, m_DrawListAnimated[i].Mesh, m_DrawListAnimated[i].SubmeshIndex, m_DrawListAnimated[i].Material);
        }

        // Render static meshes
        commandBuffer->SetGraphicsPipeline(m_GeometryPipeline.get());
        commandBuffer->SetGraphicsConstants(ShaderBindPoint::Frame, m_FrameCBs[currentFrameIdx].get());
        commandBuffer->SetGraphicsDescriptorTables(ShaderBindPoint::Frame, m_FrameResourceTable.GetBaseGpuDescriptor(), m_FrameSamplerTable.GetBaseGpuDescriptor());

        for (u32 i = 0; i < m_DrawList.size(); i++)
        {
            MeshPBRInstanceConstants constants = { m_DrawList[i].Transform, UINT32_MAX };
            commandBuffer->SetGraphicsConstants(ShaderBindPoint::Instance, &constants, sizeof(MeshPBRInstanceConstants) / 4);
            Renderer::RenderMesh(commandBuffer, m_DrawList[i].Mesh, m_DrawList[i].SubmeshIndex, m_DrawList[i].Material);
        }

        Renderer::EndRenderPass(commandBuffer, m_GeometryPipeline->GetFramebuffer());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::CompositePass(Ref<CommandBuffer> commandBuffer)
    {
        u32 currentFrameIdx = Renderer::GetCurrentFrameIndex();

        Renderer::BeginRenderPass(commandBuffer, m_CompositePipeline->GetFramebuffer());
        commandBuffer->SetGraphicsPipeline(m_CompositePipeline.get());
        commandBuffer->SetGraphicsConstants(ShaderBindPoint::Frame, m_FrameCBs[currentFrameIdx].get());
        Renderer::RenderFullscreenQuad(commandBuffer, m_GeometryPipeline->GetFramebuffer()->GetColorAttachment(AttachmentPoint::Color0));
        Renderer::EndRenderPass(commandBuffer, m_CompositePipeline->GetFramebuffer());
    }
}