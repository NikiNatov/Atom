#include "atompch.h"
#include "Application.h"

#include "Atom/Core/Core.h"
#include "Atom/Core/Logger.h"
#include "Atom/Core/Input.h"
#include "Atom/Renderer/EngineResources.h"
#include "Atom/Scripting/ScriptEngine.h"
#include "Atom/Physics/PhysicsEngine.h"
#include "Atom/Asset/AssetManager.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Application::Application(const ApplicationSpecification& spec)
        : m_Specification(spec)
    {
        ATOM_ENGINE_ASSERT(ms_Application == nullptr, "Application already exists!");
        ms_Application = this;

        Logger::Initialize(spec.AppLoggerSinks);

        WindowProperties properties;
        properties.Title = m_Specification.Name;
        properties.Width = m_Specification.WindowWidth;
        properties.Height = m_Specification.WindowHeight;
        properties.VSync = m_Specification.VSync;
        properties.EventCallback = ATOM_BIND_EVENT_FN(Application::OnEvent);
        m_Window = CreateScope<Window>(properties);

        m_ShaderLibrary = CreateScope<ShaderLibrary>();
        m_PipelineLibrary = CreateScope<PipelineLibrary>();

        ShaderCompiler::SetOutputDirectory("../Atom/shaders/bin");
        m_ShaderLibrary->LoadGraphicsShader("../Atom/shaders/MeshPBRShader.hlsl");
        m_ShaderLibrary->LoadGraphicsShader("../Atom/shaders/MeshPBRAnimatedShader.hlsl");
        m_ShaderLibrary->LoadGraphicsShader("../Atom/shaders/SkyBoxShader.hlsl");
        m_ShaderLibrary->LoadGraphicsShader("../Atom/shaders/ImGuiShader.hlsl");
        m_ShaderLibrary->LoadGraphicsShader("../Atom/shaders/CompositeShader.hlsl");
        m_ShaderLibrary->LoadGraphicsShader("../Atom/shaders/FullscreenQuadShader.hlsl");
        m_ShaderLibrary->LoadComputeShader("../Atom/shaders/GenerateMips.hlsl");
        m_ShaderLibrary->LoadComputeShader("../Atom/shaders/EquirectToCubeMap.hlsl");
        m_ShaderLibrary->LoadComputeShader("../Atom/shaders/CubeMapPrefilter.hlsl");
        m_ShaderLibrary->LoadComputeShader("../Atom/shaders/CubeMapIrradiance.hlsl");
        m_ShaderLibrary->LoadComputeShader("../Atom/shaders/BRDFShader.hlsl");

        // Initialize engine systems
        PhysicsEngine::Initialize();
        Input::Initialize(m_Window->GetWindowHandle());
        EngineResources::Initialize();

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Application::~Application()
    {
        Device::Get().WaitIdle();

        for (auto layer : m_LayerStack)
            layer->OnDetach();

        AssetManager::Shutdown();
        EngineResources::Shutdown();
        ScriptEngine::Shutdown();
        PhysicsEngine::Shutdown();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Application::Run()
    {
        while (m_Running)
        {
            Timestep ts = m_FrameTimer.GetElapsedTime();
            m_FPS = 1 / ts.GetSeconds();

            m_FrameTimer.Reset();

            m_Window->ProcessEvents();

            Device::Get().ProcessDeferredReleases(GetCurrentFrameIndex());

            ExecuteMainThreadQueue();

            AssetManager::UnloadUnusedAssets();

            for (auto layer : m_LayerStack)
                layer->OnUpdate(ts);

            m_ImGuiLayer->BeginFrame();

            for (auto layer : m_LayerStack)
                layer->OnImGuiRender();

            m_ImGuiLayer->EndFrame();

            m_Window->SwapBuffers();

            m_FrameTimer.Stop();
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Application::Close()
    {
        m_Running = false;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Application::PushLayer(Layer* layer)
    {
        layer->OnAttach();
        m_LayerStack.PushLayer(layer);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Application::PushOverlay(Layer* overlay)
    {
        overlay->OnAttach();
        m_LayerStack.PushOverlay(overlay);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Application::OnEvent(Event& event)
    {
        ScriptEngine::OnEvent(event);

        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowClosedEvent>(ATOM_BIND_EVENT_FN(Application::OnWindowClosed));

        for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
        {
            if (event.Handled)
                break;

            (*--it)->OnEvent(event);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Application::SubmitForMainThreadExecution(const std::function<void()>& function)
    {
        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);
        m_MainThreadQueue.push_back(function);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool Application::OnWindowClosed(WindowClosedEvent& event)
    {
        Close();
        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Application::ExecuteMainThreadQueue()
    {
        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

        for (auto& fn : m_MainThreadQueue)
            fn();

        m_MainThreadQueue.clear();
    }
}
