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
        m_CompositePipeline = pipelineLib.Get<GraphicsPipeline>("CompositePipeline");
        m_SkyBoxPipeline = pipelineLib.Get<GraphicsPipeline>("SkyBoxPipeline");
        m_FullScreenQuadPipeline = pipelineLib.Get<GraphicsPipeline>("FullscreenQuadPipeline");

        // Create materials
        m_SkyBoxMaterial = CreateRef<Material>(shaderLib.Get<GraphicsShader>("SkyBoxShader"), MaterialFlags::None);
        m_CompositeMaterial = CreateRef<Material>(shaderLib.Get<GraphicsShader>("CompositeShader"), MaterialFlags::None);
        m_FullScreenQuadMaterial = CreateRef<Material>(shaderLib.Get<GraphicsShader>("FullscreenQuadShader"), MaterialFlags::None);

        // Create per-frame resources
        u32 framesInFlight = Renderer::GetFramesInFlight();
        m_LightsData.resize(framesInFlight);
        m_LightsSBs.resize(framesInFlight);

        m_CameraData.resize(framesInFlight);
        m_CameraCBs.resize(framesInFlight);

        BufferDescription cbDesc;
        cbDesc.ElementCount = 1;
        cbDesc.ElementSize = sizeof(CameraCB);
        cbDesc.IsDynamic = true;

        for(u32 frameIdx = 0 ; frameIdx < framesInFlight; frameIdx++)
            m_CameraCBs[frameIdx] = CreateRef<ConstantBuffer>(cbDesc, fmt::format("TransformCB[{}]", frameIdx).c_str());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::BeginScene(Camera& camera, const glm::mat4& cameraTransform, const Ref<LightEnvironment>& lightEnvironment)
    {
        u32 currentFrameIdx = Renderer::GetCurrentFrameIndex();

        // Set camera transforms
        m_CameraData[currentFrameIdx].ProjectionMatrix = camera.GetProjection();
        m_CameraData[currentFrameIdx].ViewMatrix = glm::inverse(cameraTransform);
        m_CameraData[currentFrameIdx].CameraPosition = cameraTransform[3];

        // Set lights data
        m_LightsData[currentFrameIdx] = lightEnvironment;

        const auto& lights = m_LightsData[currentFrameIdx]->GetLights();

        if (!lights.empty())
        {
            if (!m_LightsSBs[currentFrameIdx] || m_LightsSBs[currentFrameIdx]->GetElementCount() != lights.size())
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

        void* data = m_CameraCBs[currentFrameIdx]->Map(0, 0);
        memcpy(data, &m_CameraData[currentFrameIdx], sizeof(CameraCB));
        m_CameraCBs[currentFrameIdx]->Unmap();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::BeginScene(EditorCamera& editorCamera, const Ref<LightEnvironment>& lightEnvironment)
    {
        u32 currentFrameIdx = Renderer::GetCurrentFrameIndex();

        // Set camera transforms
        m_CameraData[currentFrameIdx].ProjectionMatrix = editorCamera.GetProjection();
        m_CameraData[currentFrameIdx].ViewMatrix = editorCamera.GetViewMatrix();
        m_CameraData[currentFrameIdx].CameraPosition = editorCamera.GetPosition();

        // Set lights data
        m_LightsData[currentFrameIdx] = lightEnvironment;

        const auto& lights = m_LightsData[currentFrameIdx]->GetLights();

        if (!lights.empty())
        {
            if (!m_LightsSBs[currentFrameIdx] || m_LightsSBs[currentFrameIdx]->GetElementCount() != lights.size())
            {
                BufferDescription sbDesc;
                sbDesc.ElementCount = lights.size();
                sbDesc.ElementSize = sizeof(Light);
                sbDesc.IsDynamic = true;

                m_LightsSBs[currentFrameIdx] = CreateRef<StructuredBuffer>(sbDesc, "LightsSB");
            }

            void* lightsData = m_LightsSBs[currentFrameIdx]->Map(0, 0);
            memcpy(lightsData, lights.data(), sizeof(Light) * lights.size());
            m_LightsSBs[currentFrameIdx]->Unmap();
        }

        void* data = m_CameraCBs[currentFrameIdx]->Map(0, 0);
        memcpy(data, &m_CameraData[currentFrameIdx], sizeof(CameraCB));
        m_CameraCBs[currentFrameIdx]->Unmap();
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
            Ref<Material> material = materialTable && materialTable->HasMaterial(submeshIdx) ? materialTable->GetMaterial(submeshIdx) : meshMaterialTable->GetMaterial(submeshIdx);

            DrawCommand& drawCommand = m_DrawList.emplace_back();
            drawCommand.Mesh = mesh;
            drawCommand.SubmeshIndex = submeshIdx;
            drawCommand.Transform = transform;
            drawCommand.Material = material ? material : Renderer::GetErrorMaterial();
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::OnViewportResize(u32 width, u32 height)
    {
        m_GeometryPipeline->GetFramebuffer()->Resize(width, height);
        m_CompositePipeline->GetFramebuffer()->Resize(width, height);
        m_FullScreenQuadPipeline->GetFramebuffer()->Resize(width, height);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::PreRender()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::Flush()
    {
        CommandQueue* gfxQueue = Device::Get().GetCommandQueue(CommandQueueType::Graphics);
        Ref<CommandBuffer> gfxCmdBuffer = gfxQueue->GetCommandBuffer();
        gfxCmdBuffer->Begin();

        PreRender();
        GeometryPass(gfxCmdBuffer);
        CompositePass(gfxCmdBuffer);

        gfxCmdBuffer->End();
        gfxQueue->ExecuteCommandList(gfxCmdBuffer);

        m_DrawList.clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::GeometryPass(Ref<CommandBuffer> commandBuffer)
    {
        u32 currentFrameIdx = Renderer::GetCurrentFrameIndex();

        Renderer::BeginRenderPass(commandBuffer, m_GeometryPipeline->GetFramebuffer());

        // Render skybox
        m_SkyBoxMaterial->SetUniform("InvViewProjMatrix", glm::inverse(m_CameraData[currentFrameIdx].ProjectionMatrix * m_CameraData[currentFrameIdx].ViewMatrix));
        m_SkyBoxMaterial->SetTexture("EnvironmentMap", m_LightsData[currentFrameIdx]->GetEnvironmentMap());
        Renderer::RenderFullscreenQuad(commandBuffer, m_SkyBoxPipeline, nullptr, m_SkyBoxMaterial);

        // Render meshes
        for (auto& drawCommand : m_DrawList)
        {
            // TODO: These should not be set by the material
            drawCommand.Material->SetTexture("_EnvironmentMap", m_LightsData[currentFrameIdx]->GetEnvironmentMap());
            drawCommand.Material->SetTexture("_IrradianceMap", m_LightsData[currentFrameIdx]->GetIrradianceMap());
            drawCommand.Material->SetTexture("_BRDFMap", Renderer::GetBRDF());
            drawCommand.Material->SetUniform("_Transform", drawCommand.Transform);
            drawCommand.Material->SetUniform("_NumLights", m_LightsSBs[currentFrameIdx] ? m_LightsSBs[currentFrameIdx]->GetElementCount() : 0);

            Renderer::RenderMesh(commandBuffer, m_GeometryPipeline, drawCommand.Mesh, drawCommand.SubmeshIndex, drawCommand.Material, m_CameraCBs[currentFrameIdx], m_LightsSBs[currentFrameIdx]);
        }

        Renderer::EndRenderPass(commandBuffer, m_GeometryPipeline->GetFramebuffer());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::CompositePass(Ref<CommandBuffer> commandBuffer)
    {
        Renderer::BeginRenderPass(commandBuffer, m_CompositePipeline->GetFramebuffer());

        m_CompositeMaterial->SetUniform("Exposure", 0.3f);
        m_CompositeMaterial->SetTexture("SceneTexture", m_GeometryPipeline->GetFramebuffer()->GetColorAttachment(AttachmentPoint::Color0));

        Renderer::RenderFullscreenQuad(commandBuffer, m_CompositePipeline, nullptr, m_CompositeMaterial);
        Renderer::EndRenderPass(commandBuffer, m_CompositePipeline->GetFramebuffer());
    }
}