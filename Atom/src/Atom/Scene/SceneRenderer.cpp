#include "atompch.h"
#include "SceneRenderer.h"

#include "Atom/Renderer/Device.h"
#include "Atom/Renderer/Material.h"

#include "Atom/Renderer/RenderGraph/RenderGraph.h"
#include "Atom/Renderer/RenderGraph/RenderPassBuilder.h"

#include "Atom/Renderer/RenderPasses/SkyBoxPass.h"
#include "Atom/Renderer/RenderPasses/GeometryPass.h"
#include "Atom/Renderer/RenderPasses/CompositePass.h"

#include <imgui.h>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    SceneRenderer::SceneRenderer(bool renderToSwapChain)
        : m_RenderToSwapChain(renderToSwapChain), m_ViewportWidth(1), m_ViewportHeight(1), m_RenderGraph(*this)
    {
        m_FrameData.resize(Renderer::GetFramesInFlight());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    SceneRenderer::~SceneRenderer()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::BeginScene(Camera& camera, const glm::mat4& cameraTransform, const Ref<LightEnvironment>& lightEnvironment)
    {
        u32 currentFrameIdx = Renderer::GetCurrentFrameIndex();

        // Set frame data
        m_FrameData[currentFrameIdx].CameraConstants.ViewMatrix = glm::inverse(cameraTransform);
        m_FrameData[currentFrameIdx].CameraConstants.ProjectionMatrix = camera.GetProjection();
        m_FrameData[currentFrameIdx].CameraConstants.InvViewProjMatrix = glm::inverse(camera.GetProjection() * m_FrameData[currentFrameIdx].CameraConstants.ViewMatrix);
        m_FrameData[currentFrameIdx].CameraConstants.CameraPosition = cameraTransform[3];
        m_FrameData[currentFrameIdx].CameraConstants.CameraExposure = 0.5f; // Hard-coded for now
        m_FrameData[currentFrameIdx].Lights = lightEnvironment;
        m_FrameData[currentFrameIdx].BoneTransforms.clear();
        m_FrameData[currentFrameIdx].StaticMeshes.clear();
        m_FrameData[currentFrameIdx].AnimatedMeshes.clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::BeginScene(EditorCamera& editorCamera, const Ref<LightEnvironment>& lightEnvironment)
    {
        u32 currentFrameIdx = Renderer::GetCurrentFrameIndex();

        // Set camera transforms
        m_FrameData[currentFrameIdx].CameraConstants.ViewMatrix = editorCamera.GetViewMatrix();
        m_FrameData[currentFrameIdx].CameraConstants.ProjectionMatrix = editorCamera.GetProjection();
        m_FrameData[currentFrameIdx].CameraConstants.InvViewProjMatrix = glm::inverse(editorCamera.GetProjection() * editorCamera.GetViewMatrix());
        m_FrameData[currentFrameIdx].CameraConstants.CameraPosition = editorCamera.GetPosition();
        m_FrameData[currentFrameIdx].CameraConstants.CameraExposure = 0.5f; // Hard-coded for now
        m_FrameData[currentFrameIdx].Lights = lightEnvironment;
        m_FrameData[currentFrameIdx].BoneTransforms.clear();
        m_FrameData[currentFrameIdx].StaticMeshes.clear();
        m_FrameData[currentFrameIdx].AnimatedMeshes.clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<MaterialTable>& materialTable)
    {
        if (!mesh)
            return;

        u32 currentFrameIdx = Renderer::GetCurrentFrameIndex();

        const auto& submeshes = mesh->GetSubmeshes();
        for (u32 submeshIdx = 0 ; submeshIdx < submeshes.size(); submeshIdx++)
        {
            const Submesh& submesh = submeshes[submeshIdx];
            const Ref<MaterialTable>& meshMaterialTable = mesh->GetMaterialTable();
            Ref<MaterialAsset> material = materialTable && materialTable->HasMaterial(submesh.MaterialIndex) ? materialTable->GetMaterial(submesh.MaterialIndex) : meshMaterialTable->GetMaterial(submesh.MaterialIndex);

            MeshEntry& meshEntry = m_FrameData[currentFrameIdx].StaticMeshes.emplace_back();
            meshEntry.Mesh = mesh;
            meshEntry.SubmeshIndex = submeshIdx;
            meshEntry.Transform = transform;
            meshEntry.Material = material ? material->GetResource() : Renderer::GetErrorMaterial();
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
            Ref<MaterialAsset> material = materialTable && materialTable->HasMaterial(submesh.MaterialIndex) ? materialTable->GetMaterial(submesh.MaterialIndex) : meshMaterialTable->GetMaterial(submesh.MaterialIndex);

            MeshEntry& meshEntry = m_FrameData[currentFrameIdx].AnimatedMeshes.emplace_back();
            meshEntry.Mesh = mesh;
            meshEntry.SubmeshIndex = submeshIdx;
            meshEntry.Transform = transform;
            meshEntry.Material = material ? material->GetResource() : Renderer::GetErrorMaterialAnimated();
            meshEntry.BoneTransformIndex = m_FrameData[currentFrameIdx].BoneTransforms.size();
        }

        // Set all bone transforms
        for (auto& bone : skeleton->GetBones())
            m_FrameData[currentFrameIdx].BoneTransforms.push_back(bone.AnimatedTransform);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::SetViewportSize(u32 width, u32 height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::PreRender(Ref<CommandBuffer> commandBuffer)
    {
        // TODO: sorting, culling, ...
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::Render()
    {
        m_RenderGraph.Reset();

        m_RenderGraph.AddRenderPass<SkyBoxPass>("SkyBoxPass", m_ViewportWidth, m_ViewportHeight);
        m_RenderGraph.AddRenderPass<GeometryPass>("GeometryPass");
        m_RenderGraph.AddRenderPass<CompositePass>("CompositePass", m_ViewportWidth, m_ViewportHeight, m_RenderToSwapChain);

        m_RenderGraph.Build();
        m_RenderGraph.Execute();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneRenderer::OnImGuiRender()
    {
        ImGui::Begin("Scene Renderer");

        for (RenderPassID passID : m_RenderGraph.GetOrderedPasses())
        {
            if (ImGui::CollapsingHeader(m_RenderGraph.GetRenderPass(passID)->GetName().c_str()))
            {
                const ResourceScheduler& resourceScheduler = m_RenderGraph.GetResourceScheduler();
                for (const IResourceView* outputView : resourceScheduler.GetPassOutputs(passID))
                {
                    Resource* resource = resourceScheduler.GetResource(outputView->GetResourceID());

                    ImGui::Columns(2);
                    ImGui::SetColumnWidth(0, 150.0f);
                    ImGui::Text(resource->GetName());
                    ImGui::NextColumn();
                    ImGui::PushItemWidth(-1);

                    if (resource->As<TextureResource>() || resource->As<RenderSurfaceResource>())
                    {
                        ImGui::ImageButton((ImTextureID)resource->GetHWResource(), { 256.0f, 256.0f }, { 0.0f, 0.0f });
                    }

                    ImGui::PopItemWidth();
                    ImGui::Columns(1);

                    ImGui::Separator();
                }
            }
        }

        ImGui::End();
    }
}