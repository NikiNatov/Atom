#include "RuntimeLayer.h"

#include "Atom/Project/Project.h"
#include "Atom/Core/Application.h"

#include <glm/glm.hpp>
#include <imgui.h>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    RuntimeLayer::RuntimeLayer()
        : Layer("RuntimeLayer")
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RuntimeLayer::~RuntimeLayer()
    {
        
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RuntimeLayer::OnAttach()
    {
        const auto& cmdLineArgs = Application::Get().GetSpecification().CommandLineArgs;
        OpenProject(cmdLineArgs.Count > 1 ? cmdLineArgs.Args[1] : "../AtomEditor/SandboxProject/SandboxProject.atmproj");

        const auto& window = Application::Get().GetWindow();

        RendererSpecification rendererSpec;
        rendererSpec.RenderToSwapChain = true;
        m_SceneRenderer = CreateRef<Renderer>(rendererSpec);
        m_SceneRenderer->SetViewportSize(window.GetWidth(), window.GetHeight());

        m_ActiveScene->OnStart();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RuntimeLayer::OnDetach()
    {
        m_ActiveScene->OnStop();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RuntimeLayer::OnUpdate(Timestep ts)
    {
        m_ActiveScene->OnUpdate(ts);
        m_ActiveScene->OnRuntimeRender(m_SceneRenderer);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RuntimeLayer::OnImGuiRender()
    {
        Window& window = Application::Get().GetWindow();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowPos(ImVec2{ 0.0f, 0.0f });
        ImGui::SetNextWindowSize(ImVec2{ (f32)window.GetWidth(), (f32)window.GetHeight() });
        ImGui::Begin("GameUI", 0, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

        ImVec2 startPos = ImGui::GetCursorPos();

        ImVec2 fpsTextPos = startPos;
        fpsTextPos.x += 5.0f;
        fpsTextPos.y += 5.0f;

        u32 fps = Application::Get().GetFPS();
        ImVec4 fpsTextColor;

        if (fps < 20)
            fpsTextColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        else if (fps >= 20 && fps < 30)
            fpsTextColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
        else
            fpsTextColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

        ImGui::SetCursorPos(fpsTextPos);
        ImGui::PushStyleColor(ImGuiCol_Text, fpsTextColor);
        ImGui::Text("FPS: %d", fps);
        ImGui::PopStyleColor();

        ImGui::SetCursorPos(startPos);
        m_ActiveScene->OnImGuiRender();
        ImGui::End();
        ImGui::PopStyleVar();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RuntimeLayer::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>(ATOM_BIND_EVENT_FN(RuntimeLayer::OnKeyPressed));
        dispatcher.Dispatch<WindowResizedEvent>(ATOM_BIND_EVENT_FN(RuntimeLayer::OnWindowResized));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RuntimeLayer::OpenProject(const std::filesystem::path& filepath)
    {
        if (!Project::OpenProject(filepath))
        {
            ATOM_ERROR("Failed opening project \"{}\"", filepath);
            return;
        }

        UUID sceneUUID = AssetManager::GetUUIDForAssetPath(Project::GetActiveProject()->GetSettings().StartScenePath);

        if (sceneUUID == 0)
        {
            ATOM_ERROR("Failed loading scene \"{}\"", Project::GetActiveProject()->GetSettings().StartScenePath);
            return;
        }

        m_ActiveScene = AssetManager::GetAsset<Scene>(sceneUUID, true);
        m_ActiveScene->OnViewportResize(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool RuntimeLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        return false;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool RuntimeLayer::OnWindowResized(WindowResizedEvent& e)
    {
        m_ActiveScene->OnViewportResize(e.GetWidth(), e.GetHeight());
        m_SceneRenderer->SetViewportSize(e.GetWidth(), e.GetHeight());
        return false;
    }
}