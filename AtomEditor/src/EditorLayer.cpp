#include "EditorLayer.h"
#include "EditorResources.h"
#include "Panels/ConsolePanel.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <imgui/imgui.h>

namespace Atom
{
    struct CameraCB
    {
        glm::mat4 ViewMatrix = glm::mat4(1.0f);
        glm::mat4 ProjMatrix = glm::mat4(1.0f);
        glm::vec3 CameraPosition = glm::vec3(0.0f);
        f32 p[29]{ 0 };
    };

    // -----------------------------------------------------------------------------------------------------------------------------
    EditorLayer::EditorLayer()
        : Layer("EditorLayer")
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    EditorLayer::~EditorLayer()
    {

    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::OnAttach()
    {
        Application::Get().GetImGuiLayer().SetClearRenderTarget(true);

        // Set the pipelines
        m_GeometryPipeline = Renderer::GetPipelineLibrary().Get<GraphicsPipeline>("GeometryPipeline");
        m_SkyBoxPipeline = Renderer::GetPipelineLibrary().Get<GraphicsPipeline>("SkyBoxPipeline");

        // Load the test mesh
        m_TestMesh = CreateRef<Mesh>("assets/meshes/x-wing/x-wing.gltf");

        // Create camera constant buffer
        BufferDescription cbDesc;
        cbDesc.ElementCount = 1;
        cbDesc.ElementSize = sizeof(CameraCB);
        cbDesc.IsDynamic = true;

        m_CameraCB = CreateRef<ConstantBuffer>(cbDesc, "CameraCB");

        // Create environment map
        m_EnvironmentMap = Renderer::CreateEnvironmentMap("assets/environments/the_sky_is_on_fire_4k.hdr");
        m_SkyBoxMaterial = CreateRef<Material>(Renderer::GetShaderLibrary().Get<GraphicsShader>("SkyBoxShader"), MaterialFlags::None, "SkyBoxMaterial");
        m_SkyBoxMaterial->SetTexture("EnvironmentMap", m_EnvironmentMap.first);

        EditorResources::Initialize();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::OnDetach()
    {
        EditorResources::Shutdown();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::OnUpdate(Timestep ts)
    {
        m_Camera.OnUpdate(ts);

        CameraCB cameraCB;
        cameraCB.ProjMatrix = m_Camera.GetProjectionMatrix();
        cameraCB.ViewMatrix = m_Camera.GetViewMatrix();
        cameraCB.CameraPosition = m_Camera.GetPosition();

        void* data = m_CameraCB->Map(0, 0);
        memcpy(data, &cameraCB, sizeof(CameraCB));
        m_CameraCB->Unmap();

        CommandQueue* gfxQueue = Device::Get().GetCommandQueue(CommandQueueType::Graphics);
        Ref<CommandBuffer> commandBuffer = gfxQueue->GetCommandBuffer();
        commandBuffer->Begin();

        Renderer::BeginRenderPass(commandBuffer.get(), m_GeometryPipeline->GetFramebuffer());

        // Render skybox
        m_SkyBoxMaterial->SetUniform("InvViewProjMatrix", glm::inverse(cameraCB.ViewMatrix * cameraCB.ProjMatrix));
        Renderer::RenderFullscreenQuad(commandBuffer.get(), m_SkyBoxPipeline.get(), nullptr, m_SkyBoxMaterial.get());

        // Render mesh
        Renderer::RenderGeometry(commandBuffer.get(), m_GeometryPipeline.get(), m_TestMesh.get(), m_CameraCB.get());
        Renderer::EndRenderPass(commandBuffer.get(), m_GeometryPipeline->GetFramebuffer());

        commandBuffer->End();

        gfxQueue->ExecuteCommandList(commandBuffer);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::OnImGuiRender()
    {
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", 0, window_flags);
        ImGui::PopStyleVar();

        ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("EditorDockspace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New", "Ctrl+N"))
                {
                }

                if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
                {
                }

                if (ImGui::MenuItem("Save As...", "Ctrl+S"))
                {
                }

                if (ImGui::MenuItem("Exit"))
                    Application::Get().Close();

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ConsolePanel::OnImGuiRender();
        ImGui::ShowDemoWindow(false);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
        ImVec2 panelSize = ImGui::GetContentRegionAvail();

        if (m_ViewportSize.x != panelSize.x || m_ViewportSize.y != panelSize.y)
        {
            m_ViewportSize = { panelSize.x, panelSize.y };
            m_GeometryPipeline->GetFramebuffer()->Resize(m_ViewportSize.x, m_ViewportSize.y);
            m_Camera.SetViewport(m_ViewportSize.x, m_ViewportSize.y);
        }

        const RenderTexture2D* sceneTexture = m_GeometryPipeline->GetFramebuffer()->GetColorAttachment(AttachmentPoint::Color0);
        ImGui::Image((ImTextureID)sceneTexture, { (f32)sceneTexture->GetWidth(), (f32)sceneTexture->GetHeight() });

        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::End();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::OnEvent(Event& event)
    {
        m_Camera.OnEvent(event);
    }
}