#include "EditorLayer.h"
#include "EditorResources.h"
#include "Panels/ConsolePanel.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <imgui/imgui.h>

namespace Atom
{
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
        EditorResources::Initialize();

        auto environment = Renderer::CreateEnvironmentMap("assets/environments/GCanyon_C_YumaPoint_3k.hdr");

        m_Scene = CreateRef<Scene>("TestScene");

        {
            Entity camera = m_Scene->CreateEntity("Camera");
            auto& cc = camera.AddComponent<CameraComponent>();
            cc.Primary = true;

            auto& sc = camera.AddComponent<ScriptComponent>();
            sc.ScriptClass = "Camera";

            camera.GetComponent<TransformComponent>().Translation.z = 3.0f;
        }

        {
            Entity sphere = m_Scene->CreateEntity("Player");
            sphere.AddComponent<MeshComponent>(CreateRef<Mesh>("assets/meshes/sphere.gltf"));

            auto& sc = sphere.AddComponent<ScriptComponent>();
            sc.ScriptClass = "Player";
        }

        {
            Entity skyLight = m_Scene->CreateEntity("SkyLight");
            auto& slc = skyLight.AddComponent<SkyLightComponent>();
            slc.EnvironmentMap = environment.first;
            slc.IrradianceMap = environment.second;
        }

        {
            Entity dirLight = m_Scene->CreateEntity("DirLight");
            dirLight.GetComponent<TransformComponent>().Translation = { 5.0f, 5.0f, 0.0f };
            auto& dlc = dirLight.AddComponent<DirectionalLightComponent>();
            dlc.Color = { 1.0f, 0.0f, 0.0f };
            dlc.Intensity = 5.0f;
        }
        
        {
            Entity pointLight = m_Scene->CreateEntity("PointLight");
            pointLight.GetComponent<TransformComponent>().Translation = { -2.0f, 2.0f, 0.0f };
            auto& plc = pointLight.AddComponent<PointLightComponent>();
            plc.Color = { 0.0f, 1.0f, 0.0f };
            plc.Intensity = 15.0f;
            plc.AttenuationFactors = { 1.0f, 0.08f, 0.0f };
        }

        {
            Entity spotLight = m_Scene->CreateEntity("SpotLight");
            spotLight.GetComponent<TransformComponent>().Translation = { 0.0f, 0.0f, 5.0f };
            auto& slc = spotLight.AddComponent<SpotLightComponent>();
            slc.Color = { 0.0f, 0.0f, 1.0f };
            slc.Direction = { 0.0f, 0.0f, -1.0f };
            slc.ConeAngle = 2.0f;
            slc.Intensity = 15.0f;
            slc.AttenuationFactors = { 1.0f, 0.08f, 0.0f };
        }

        m_Renderer = CreateRef<SceneRenderer>();
        m_Renderer->Initialize();

        m_SceneHierarchyPanel.SetScene(m_Scene);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::OnDetach()
    {
        EditorResources::Shutdown();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::OnUpdate(Timestep ts)
    {
        if (m_NeedsResize)
        {
            m_Scene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
            m_Renderer->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
            m_NeedsResize = false;
        }

        switch (m_Scene->GetSceneState())
        {
            case SceneState::Edit:
            {
                m_Scene->GetEditorCamera().OnUpdate(ts);
                m_Scene->OnEditRender(m_Renderer);
                break;
            }
            case SceneState::Running:
            {
                m_Scene->OnUpdate(ts);
                m_Scene->OnRuntimeRender(m_Renderer);
                break;
            }
        }
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

        // Render panels
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
        ImGui::Begin("##SceneControl", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
        ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });

        f32 buttonSize = ImGui::GetWindowHeight() - 4.0f;
        ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (buttonSize * 0.5f) * 2.0f);

        if (ImGui::ImageButton((ImTextureID)EditorResources::ScenePlayIcon.get(), { buttonSize, buttonSize }, { 0, 1 }, { 1, 0 }))
        {
            if (m_Scene->GetSceneState() != SceneState::Running)
            {
                m_Scene->SetSceneState(SceneState::Running);
                m_Scene->OnStart();
            }
        }
        ImGui::SameLine();
        if (ImGui::ImageButton((ImTextureID)EditorResources::SceneStopIcon.get(), { buttonSize, buttonSize }, { 0, 1 }, { 1, 0 }))
        {
            if (m_Scene->GetSceneState() != SceneState::Edit)
            {
                m_Scene->SetSceneState(SceneState::Edit);
                m_Scene->OnStop();
            }
        }
        ImGui::PopStyleColor();
        ImGui::End();
        ImGui::PopStyleVar();

        ConsolePanel::OnImGuiRender();
        m_SceneHierarchyPanel.OnImGuiRender();

        m_EntityInspectorPanel.SetEntity(m_SceneHierarchyPanel.GetSelectedEntity());
        m_EntityInspectorPanel.OnImGuiRender();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
        ImVec2 panelSize = ImGui::GetContentRegionAvail();

        if (m_ViewportSize.x != panelSize.x || m_ViewportSize.y != panelSize.y)
        {
            m_ViewportSize = { panelSize.x, panelSize.y };
            m_NeedsResize = true;
        }

        Ref<RenderTexture2D> finalImage = m_Renderer->GetFinalImage();
        ImGui::Image((ImTextureID)finalImage.get(), {(f32)finalImage->GetWidth(), (f32)finalImage->GetHeight()});

        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::End();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::OnEvent(Event& event)
    {
    }
}