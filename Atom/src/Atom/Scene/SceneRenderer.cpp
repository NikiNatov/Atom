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

        // Create transform constant buffer
        BufferDescription cbDesc;
        cbDesc.ElementCount = 1;
        cbDesc.ElementSize = sizeof(TransformCB);
        cbDesc.IsDynamic = true;

        m_TransformCB = CreateRef<ConstantBuffer>(cbDesc, "TransformCB");
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::BeginScene(Camera& camera, const glm::mat4& cameraTransform, const Ref<LightEnvironment>& lightEnvironment)
    {
        // Set camera transforms
        m_TransformData.ProjectionMatrix = camera.GetProjection();
        m_TransformData.ViewMatrix = glm::inverse(cameraTransform);
        m_TransformData.CameraPosition = cameraTransform[3];

        // Set lights data
        m_Environment = lightEnvironment;

        const auto& lights = m_Environment->GetLights();

        if (!m_LightsSB || m_LightsSB->GetElementCount() != lights.size())
        {
            BufferDescription sbDesc;
            sbDesc.ElementCount = lights.size();
            sbDesc.ElementSize = sizeof(Light);
            sbDesc.IsDynamic = true;

            m_LightsSB = CreateRef<StructuredBuffer>(sbDesc, "LightsSB");
        }

        void* lightsData = m_LightsSB->Map(0, 0);
        memcpy(lightsData, lights.data(), sizeof(Light) * lights.size());
        m_LightsSB->Unmap();

        void* data = m_TransformCB->Map(0, 0);
        memcpy(data, &m_TransformData, sizeof(TransformCB));
        m_TransformCB->Unmap();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::BeginScene(EditorCamera& editorCamera, const Ref<LightEnvironment>& lightEnvironment)
    {
        // Set camera transforms
        m_TransformData.ProjectionMatrix = editorCamera.GetProjection();
        m_TransformData.ViewMatrix = editorCamera.GetViewMatrix();
        m_TransformData.CameraPosition = editorCamera.GetPosition();

        // Set lights data
        m_Environment = lightEnvironment;

        const auto& lights = m_Environment->GetLights();

        if (!m_LightsSB || m_LightsSB->GetElementCount() != lights.size())
        {
            BufferDescription sbDesc;
            sbDesc.ElementCount = lights.size();
            sbDesc.ElementSize = sizeof(Light);
            sbDesc.IsDynamic = true;

            m_LightsSB = CreateRef<StructuredBuffer>(sbDesc, "LightsSB");
        }

        void* lightsData = m_LightsSB->Map(0, 0);
        memcpy(lightsData, lights.data(), sizeof(Light) * lights.size());
        m_LightsSB->Unmap();

        void* data = m_TransformCB->Map(0, 0);
        memcpy(data, &m_TransformData, sizeof(TransformCB));
        m_TransformCB->Unmap();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const MaterialTable& materialTable)
    {
        if (!mesh)
            return;

        const auto& submeshes = mesh->GetSubmeshes();

        for (u32 submeshIdx = 0 ; submeshIdx < submeshes.size(); submeshIdx++)
        {
            const Submesh& submesh = submeshes[submeshIdx];
            const MaterialTable& meshMaterialTable = mesh->GetMaterialTable();
            Ref<MaterialAsset> materialAsset = materialTable.HasMaterial(submeshIdx) ? materialTable.GetMaterial(submeshIdx) : meshMaterialTable.GetMaterial(submeshIdx);

            DrawCommand& drawCommand = m_DrawList.emplace_back();
            drawCommand.Mesh = mesh;
            drawCommand.SubmeshIndex = submeshIdx;
            drawCommand.Transform = transform;
            drawCommand.Material = materialAsset ? materialAsset->GetMaterial() : Renderer::GetErrorMaterial();
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

        GeometryPass(gfxCmdBuffer);
        CompositePass(gfxCmdBuffer);

        gfxCmdBuffer->End();
        gfxQueue->ExecuteCommandList(gfxCmdBuffer);

        m_DrawList.clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::GeometryPass(Ref<CommandBuffer> commandBuffer)
    {
        Renderer::BeginRenderPass(commandBuffer, m_GeometryPipeline->GetFramebuffer());

        // Render skybox
        m_SkyBoxMaterial->SetUniform("InvViewProjMatrix", glm::inverse(m_TransformData.ProjectionMatrix * m_TransformData.ViewMatrix));
        m_SkyBoxMaterial->SetTexture("EnvironmentMap", m_Environment->GetEnvironmentMap());
        Renderer::RenderFullscreenQuad(commandBuffer, m_SkyBoxPipeline, nullptr, m_SkyBoxMaterial);

        // Render meshes
        for (auto& drawCommand : m_DrawList)
        {
            // TODO: These should not be set by the material
            drawCommand.Material->SetTexture("EnvironmentMap", m_Environment->GetEnvironmentMap());
            drawCommand.Material->SetTexture("IrradianceMap", m_Environment->GetIrradianceMap());
            drawCommand.Material->SetTexture("BRDFMap", Renderer::GetBRDF());
            drawCommand.Material->SetUniform("Transform", drawCommand.Transform);
            drawCommand.Material->SetUniform("NumLights", m_LightsSB->GetElementCount());

            Renderer::RenderMesh(commandBuffer, m_GeometryPipeline, drawCommand.Mesh, drawCommand.SubmeshIndex, drawCommand.Material, m_TransformCB, m_LightsSB);
        }

        Renderer::EndRenderPass(commandBuffer, m_GeometryPipeline->GetFramebuffer());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::CompositePass(Ref<CommandBuffer> commandBuffer)
    {
        Renderer::BeginRenderPass(commandBuffer, m_CompositePipeline->GetFramebuffer());

        m_CompositeMaterial->SetUniform("Exposure", 1.0f);
        m_CompositeMaterial->SetTexture("SceneTexture", m_GeometryPipeline->GetFramebuffer()->GetColorAttachment(AttachmentPoint::Color0));

        Renderer::RenderFullscreenQuad(commandBuffer, m_CompositePipeline, nullptr, m_CompositeMaterial);
        Renderer::EndRenderPass(commandBuffer, m_CompositePipeline->GetFramebuffer());
    }
}