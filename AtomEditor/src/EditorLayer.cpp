#include "EditorLayer.h"
#include "EditorResources.h"
#include "Panels/ConsolePanel.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    EditorLayer::EditorLayer()
        : Layer("EditorLayer")
    {
        ms_Instance = this;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    EditorLayer::~EditorLayer()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::OnAttach()
    {
        EditorResources::Initialize();

        auto environment = Renderer::CreateEnvironmentMap("TestProject/Assets/Textures/GCanyon_C_YumaPoint_3k.hdr");

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
            Entity player = m_Scene->CreateEntity("Player");
            player.AddComponent<MeshComponent>(CreateRef<Mesh>("TestProject/Assets/Meshes/cube.gltf"));

            auto& sc = player.AddComponent<ScriptComponent>();
            sc.ScriptClass = "Player";

            auto& rbc = player.AddComponent<RigidbodyComponent>();
            rbc.Type = RigidbodyComponent::RigidbodyType::Dynamic;

            auto& bcc = player.AddComponent<BoxColliderComponent>();
            bcc.Restitution = 1.0f;
        }

        {
            Entity ground = m_Scene->CreateEntity("Ground");
            ground.AddComponent<MeshComponent>(CreateRef<Mesh>("TestProject/Assets/Meshes/cube.gltf"));

            ground.GetComponent<TransformComponent>().Scale = { 3.0f, 1.0f, 3.0f };
            ground.GetComponent<TransformComponent>().Translation.y = -3.0f;

            auto& rbc = ground.AddComponent<RigidbodyComponent>();
            rbc.Type = RigidbodyComponent::RigidbodyType::Static;

            auto& bcc = ground.AddComponent<BoxColliderComponent>();
            bcc.Size = { 3.0f, 1.0f, 3.0f };
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
        m_GuizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
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
                    NewScene();
                }

                if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
                {
                    OpenScene();
                }

                if (ImGui::MenuItem("Save As...", "Ctrl+S"))
                {
                    SaveScene();
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

        Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
        m_EntityInspectorPanel.SetEntity(selectedEntity);
        m_EntityInspectorPanel.OnImGuiRender();

        // Render scene viewport
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
        ImVec2 panelSize = ImGui::GetContentRegionAvail();

        Application::Get().GetImGuiLayer().SetBlockEvents(!ImGui::IsWindowFocused() && !ImGui::IsWindowHovered());

        if (m_ViewportSize.x != panelSize.x || m_ViewportSize.y != panelSize.y)
        {
            m_ViewportSize = { panelSize.x, panelSize.y };
            m_NeedsResize = true;
        }

        Ref<RenderTexture2D> finalImage = m_Renderer->GetFinalImage();
        ImGui::Image((ImTextureID)finalImage.get(), {(f32)finalImage->GetWidth(), (f32)finalImage->GetHeight()});

        // Render guizmos
        if (selectedEntity && m_Scene->GetSceneState() == SceneState::Edit && m_GuizmoOperation != -1)
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

            auto& viewMatrix = m_Scene->GetEditorCamera().GetViewMatrix();
            auto& projMatrix = m_Scene->GetEditorCamera().GetProjection();

            f32 snapValue = m_GuizmoOperation == ImGuizmo::OPERATION::ROTATE ? 45.0f : 0.5f;

            f32 snap[3] = { snapValue, snapValue, snapValue };
            m_GuizmoSnap = Input::IsKeyPressed(Key::LCtrl);

            auto& tc = selectedEntity.GetComponent<TransformComponent>();
            glm::mat4 entityTransform = tc.GetTransform();
            glm::mat4 entityWorldTransform = entityTransform;
            glm::mat4 parentWorldTransform = glm::mat4(1.0f);

            if (selectedEntity.GetComponent<SceneHierarchyComponent>().Parent)
            {
                Entity currentParent = m_Scene->FindEntityByUUID(selectedEntity.GetComponent<SceneHierarchyComponent>().Parent);
                while (currentParent)
                {
                    glm::mat4 parentTransform = currentParent.GetComponent<TransformComponent>().GetTransform();
                    entityWorldTransform = parentTransform * entityWorldTransform;
                    currentParent = m_Scene->FindEntityByUUID(currentParent.GetComponent<SceneHierarchyComponent>().Parent);
                }

                parentWorldTransform = entityWorldTransform * glm::inverse(entityTransform);
            }

            ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projMatrix), 
                (ImGuizmo::OPERATION)m_GuizmoOperation, ImGuizmo::LOCAL, glm::value_ptr(entityWorldTransform), NULL, m_GuizmoSnap ? &snap[0] : NULL);

            entityTransform = glm::inverse(parentWorldTransform) * entityWorldTransform;

            if (ImGuizmo::IsUsing())
            {
                glm::vec3 translation, rotation, scale;
                ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(entityTransform), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));

                glm::vec3 deltaRotation = glm::radians(rotation) - tc.Rotation;

                tc.Translation = translation;
                //tc.Rotation = glm::radians(rotation);
                tc.Rotation += deltaRotation;
                tc.Scale = scale;
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::End();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::OnEvent(Event& event)
    {
        if (m_Scene->GetSceneState() != SceneState::Running && !ImGuizmo::IsUsing())
            m_Scene->GetEditorCamera().OnEvent(event);

        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>(ATOM_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        if (e.GetRepeatCount() > 0)
            return false;

        bool control = Input::IsKeyPressed(Key::LCtrl) || Input::IsKeyPressed(Key::RCtrl);
        bool shift = Input::IsKeyPressed(Key::LShift) || Input::IsKeyPressed(Key::RShift);

        switch (e.GetKeyCode())
        {
            case Key::S:
            {
                if (control)
                    SaveScene();

                break;
            }
            case Key::N:
            {
                if (control)
                    NewScene();

                break;
            }
            case Key::O:
            {
                if (control)
                    OpenScene();

                break;
            }
            case Key::Q:
                m_GuizmoOperation = -1;
                break;
            case Key::T:
                m_GuizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
                break;
            case Key::R:
                m_GuizmoOperation = ImGuizmo::OPERATION::ROTATE;
                break;
            case Key::E:
                m_GuizmoOperation = ImGuizmo::OPERATION::SCALE;
                break;
        }

        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::SaveScene()
    {
        const std::filesystem::path& path = FileDialog::SaveFile("Atom Scene (*.scene)\0*.scene\0");
        if (!path.empty())
        {
            SceneSerializer serializer(m_Scene);
            serializer.Serialize(path);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::NewScene()
    {
        m_Scene = CreateRef<Scene>();
        m_Scene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
        m_SceneHierarchyPanel.SetScene(m_Scene);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::OpenScene()
    {
        const std::filesystem::path& path = FileDialog::OpenFile("Atom Scene (*.scene)\0*.scene\0");

        if (!path.empty())
        {
            m_Scene = CreateRef<Scene>();
            m_Scene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
            m_SceneHierarchyPanel.SetScene(m_Scene);

            SceneSerializer serializer(m_Scene);
            serializer.Deserialize(path);
        }
    }
}