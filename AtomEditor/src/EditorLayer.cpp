#include "EditorLayer.h"
#include "EditorResources.h"
#include "Panels/ConsolePanel.h"
#include "Panels/AssetManagerPanel.h"
#include "Dialogs/FileDialog.h"

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
        AssetManager::SetAssetFolder("TestProject/Assets");

        m_EditorScene = AssetManager::GetAsset<Scene>(ContentTools::CreateSceneAsset("TestScene"), true);

        {
            Entity camera = m_EditorScene->CreateEntity("Camera");
            auto& cc = camera.AddComponent<CameraComponent>();
            cc.Primary = true;

            auto& sc = camera.AddComponent<ScriptComponent>();
            sc.ScriptClass = "Camera";

            camera.GetComponent<TransformComponent>().Translation.z = 3.0f;
        }

        {
            UUID meshUUID = ContentTools::ImportMeshAsset("TestProject/Assets/Meshes/sphere.gltf", "TestProject/Assets/Meshes/", MeshImportSettings());

            Entity player = m_EditorScene->CreateEntity("Player");
            player.AddComponent<MeshComponent>(AssetManager::GetAsset<Mesh>(meshUUID, true));

            auto& sc = player.AddComponent<ScriptComponent>();
            sc.ScriptClass = "Player";

            auto& rbc = player.AddComponent<RigidbodyComponent>();
            rbc.Type = RigidbodyComponent::RigidbodyType::Dynamic;

            auto& bcc = player.AddComponent<BoxColliderComponent>();
            bcc.Restitution = 1.0f;
        }

        {
            UUID meshUUID = ContentTools::ImportMeshAsset("TestProject/Assets/Meshes/cube.gltf", "TestProject/Assets/Meshes/", MeshImportSettings());

            Entity ground = m_EditorScene->CreateEntity("Ground");
            ground.AddComponent<MeshComponent>(AssetManager::GetAsset<Mesh>(meshUUID, true));

            ground.GetComponent<TransformComponent>().Scale = { 3.0f, 1.0f, 3.0f };
            ground.GetComponent<TransformComponent>().Translation.y = -3.0f;

            auto& rbc = ground.AddComponent<RigidbodyComponent>();
            rbc.Type = RigidbodyComponent::RigidbodyType::Static;

            auto& bcc = ground.AddComponent<BoxColliderComponent>();
            bcc.Size = { 3.0f, 1.0f, 3.0f };
        }

        {
            TextureImportSettings importSettings;
            importSettings.Type = TextureType::TextureCube;
            importSettings.Format = TextureFormat::RGBA32F;
            UUID uuid  = ContentTools::ImportTextureAsset("TestProject/Assets/Textures/GCanyon_C_YumaPoint_3k.hdr", "TestProject/Assets/Textures", importSettings);

            Entity skyLight = m_EditorScene->CreateEntity("SkyLight");
            skyLight.AddComponent<SkyLightComponent>(AssetManager::GetAsset<TextureCube>(uuid, true));
        }

        {
            Entity dirLight = m_EditorScene->CreateEntity("DirLight");
            dirLight.GetComponent<TransformComponent>().Translation = { 5.0f, 5.0f, 0.0f };
            auto& dlc = dirLight.AddComponent<DirectionalLightComponent>();
            dlc.Color = { 1.0f, 1.0f, 1.0f };
            dlc.Intensity = 1.0f;
        }
        
        m_Renderer = CreateRef<SceneRenderer>();
        m_Renderer->Initialize();

        m_SceneHierarchyPanel.SetScene(m_EditorScene);
        m_ActiveScene = m_EditorScene;
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
            m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
            m_Renderer->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
            m_NeedsResize = false;
        }

        switch (m_ActiveScene->GetSceneState())
        {
            case SceneState::Edit:
            {
                m_ActiveScene->GetEditorCamera().OnUpdate(ts);
                m_ActiveScene->OnEditRender(m_Renderer);
                break;
            }
            case SceneState::Running:
            {
                m_ActiveScene->OnUpdate(ts);
                m_ActiveScene->OnRuntimeRender(m_Renderer);
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
                if (ImGui::MenuItem("New Scene", "Ctrl+N"))
                {
                    NewScene();
                }

                if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
                {
                    OpenScene();
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Save Scene...", "Ctrl+S"))
                {
                    SaveScene();
                }

                if (ImGui::MenuItem("Exit", "Alt+F4"))
                    Application::Get().Close();

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Assets"))
            {
                if (ImGui::BeginMenu("Import"))
                {
                    if (ImGui::MenuItem("Texture", ""))
                    {
                        m_TextureImportDialog.Open();
                    }

                    if (ImGui::MenuItem("Mesh", ""))
                    {
                        m_MeshImportDialog.Open();
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        // Render scene controls
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
        ImGui::Begin("##SceneControl", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
        ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });

        f32 buttonSize = ImGui::GetWindowHeight() - 4.0f;
        ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (buttonSize * 0.5f) * 2.0f);

        if (ImGui::ImageButton((ImTextureID)EditorResources::ScenePlayIcon.get(), { buttonSize, buttonSize }, { 0, 1 }, { 1, 0 }))
            PlayScene();

        ImGui::SameLine();

        if (ImGui::ImageButton((ImTextureID)EditorResources::SceneStopIcon.get(), { buttonSize, buttonSize }, { 0, 1 }, { 1, 0 }))
            StopScene();

        ImGui::PopStyleColor();
        ImGui::End();
        ImGui::PopStyleVar();

        // Render panels
        m_TextureImportDialog.OnImGuiRender();
        m_MeshImportDialog.OnImGuiRender();
        AssetManagerPanel::OnImGuiRender();
        ConsolePanel::OnImGuiRender();
        m_SceneHierarchyPanel.OnImGuiRender();

        Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
        m_EntityInspectorPanel.SetEntity(selectedEntity);
        m_EntityInspectorPanel.OnImGuiRender();

        // Render scene viewport
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
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
        if (selectedEntity && m_ActiveScene->GetSceneState() == SceneState::Edit && m_GuizmoOperation != -1)
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

            auto& viewMatrix = m_ActiveScene->GetEditorCamera().GetViewMatrix();
            auto& projMatrix = m_ActiveScene->GetEditorCamera().GetProjection();

            f32 snapValue = m_GuizmoOperation == ImGuizmo::OPERATION::ROTATE ? 45.0f : 0.5f;

            f32 snap[3] = { snapValue, snapValue, snapValue };
            m_GuizmoSnap = Input::IsKeyPressed(Key::LCtrl);

            auto& tc = selectedEntity.GetComponent<TransformComponent>();
            glm::mat4 entityTransform = tc.GetTransform();
            glm::mat4 entityWorldTransform = entityTransform;
            glm::mat4 parentWorldTransform = glm::mat4(1.0f);

            if (selectedEntity.GetComponent<SceneHierarchyComponent>().Parent)
            {
                Entity currentParent = m_ActiveScene->FindEntityByUUID(selectedEntity.GetComponent<SceneHierarchyComponent>().Parent);
                while (currentParent)
                {
                    glm::mat4 parentTransform = currentParent.GetComponent<TransformComponent>().GetTransform();
                    entityWorldTransform = parentTransform * entityWorldTransform;
                    currentParent = m_ActiveScene->FindEntityByUUID(currentParent.GetComponent<SceneHierarchyComponent>().Parent);
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
        if (m_ActiveScene->GetSceneState() != SceneState::Running && !ImGuizmo::IsUsing())
            m_ActiveScene->GetEditorCamera().OnEvent(event);

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
        bool alt = Input::IsKeyPressed(Key::LAlt) || Input::IsKeyPressed(Key::LAlt);

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
            case Key::D:
            {
                if (control)
                    DuplicateEntity();

                break;
            }
            case Key::F4:
            {
                if (alt)
                    Application::Get().Close();

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
        const std::filesystem::path& path = FileDialog::SaveFile("Atom Scene (*.atmscene)\0*.atmscene\0");
        if (!path.empty())
        {
            if (!AssetSerializer::Serialize(path, m_EditorScene))
            {
                ATOM_ERROR("Failed serializing scene asset {}", path);
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::NewScene()
    {
        AssetManager::UnloadAllAssets();

        m_EditorScene = AssetManager::GetAsset<Scene>(ContentTools::CreateSceneAsset(), true);
        m_EditorScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
        m_ActiveScene = m_EditorScene;

        m_SceneHierarchyPanel.SetScene(m_ActiveScene);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::OpenScene()
    {
        if (m_ActiveScene->GetSceneState() != SceneState::Edit)
            StopScene();

        const std::filesystem::path& path = FileDialog::OpenFile("Atom Scene (*.atmscene)\0*.atmscene\0");

        if (!path.empty())
        {
            UUID sceneUUID = AssetManager::GetUUIDForAssetPath(std::filesystem::relative(path));
            if (sceneUUID != 0)
            {
                AssetManager::UnloadAllAssets();

                m_EditorScene = AssetManager::GetAsset<Scene>(sceneUUID, true);
                m_EditorScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
                m_ActiveScene = m_EditorScene;

                m_SceneHierarchyPanel.SetScene(m_ActiveScene);
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::PlayScene()
    {
        if (m_ActiveScene->GetSceneState() == SceneState::Running)
            return;

        m_ActiveScene = m_EditorScene->Copy();
        m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
        m_ActiveScene->OnStart();

        m_SceneHierarchyPanel.SetScene(m_ActiveScene);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::StopScene()
    {
        if (m_ActiveScene->GetSceneState() == SceneState::Edit)
            return;

        m_ActiveScene->OnStop();
        m_ActiveScene = m_EditorScene;

        m_SceneHierarchyPanel.SetScene(m_ActiveScene);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::DuplicateEntity()
    {
        if (m_ActiveScene->GetSceneState() == SceneState::Running)
            return;

        Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();

        if (selectedEntity)
            m_ActiveScene->DuplicateEntity(selectedEntity);
    }
}