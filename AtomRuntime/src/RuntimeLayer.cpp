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
        const PipelineLibrary& pipelineLib = Renderer::GetPipelineLibrary();
        const ShaderLibrary& shaderLib = Renderer::GetShaderLibrary();

        m_SwapChainPipeline = pipelineLib.Get<GraphicsPipeline>("SwapChainPipeline");
        m_SwapChainMaterial = CreateRef<Material>(m_SwapChainPipeline->GetShader(), MaterialFlags::None);

        const auto& cmdLineArgs = Application::Get().GetSpecification().CommandLineArgs;
        OpenProject(cmdLineArgs.Count > 1 ? cmdLineArgs.Args[1] : "../AtomEditor/SandboxProject/SandboxProject.atmproj");

        m_SceneRenderer = CreateRef<SceneRenderer>();
        m_SceneRenderer->Initialize();

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

        m_SwapChainMaterial->SetTexture("SceneTexture", m_SceneRenderer->GetFinalImage());

        CommandQueue* gfxQueue = Device::Get().GetCommandQueue(CommandQueueType::Graphics);
        Ref<CommandBuffer> cmdBuffer = gfxQueue->GetCommandBuffer();
        cmdBuffer->Begin();
        Renderer::BeginRenderPass(cmdBuffer, m_SwapChainPipeline->GetFramebuffer());
        Renderer::RenderFullscreenQuad(cmdBuffer, m_SwapChainPipeline, nullptr, m_SwapChainMaterial);
        Renderer::EndRenderPass(cmdBuffer, m_SwapChainPipeline->GetFramebuffer());
        cmdBuffer->End();
        gfxQueue->ExecuteCommandList(cmdBuffer);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RuntimeLayer::OnImGuiRender()
    {
        Window& window = Application::Get().GetWindow();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowPos(ImVec2{ 0.0f, 0.0f });
        ImGui::SetNextWindowSize(ImVec2{ (f32)window.GetWidth(), (f32)window.GetHeight() });
        ImGui::Begin("GameUI", 0, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
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
        m_SceneRenderer->OnViewportResize(e.GetWidth(), e.GetHeight());
        return false;
    }
}