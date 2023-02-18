#include "atompch.h"
#include "Application.h"

#include "Atom/Core/Core.h"
#include "Atom/Core/Logger.h"
#include "Atom/Core/Input.h"
#include "Atom/Renderer/Renderer.h"
#include "Atom/Scripting/ScriptEngine.h"
#include "Atom/Physics/PhysicsEngine.h"
#include "Atom/Asset/AssetManager.h"

namespace Atom
{
    Application* Application::ms_Application = nullptr;

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

        Renderer::Initialize();
        PhysicsEngine::Initialize();
        Input::Initialize(m_Window->GetWindowHandle());

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
        Renderer::Shutdown();
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

            ExecuteMainThreadQueue();

            m_Window->ProcessEvents();

            Renderer::BeginFrame();

            AssetManager::UnloadUnusedAssets();

            if (!m_Window->IsMinimized())
            {
                for (auto layer : m_LayerStack)
                    layer->OnUpdate(ts);
            }

            m_ImGuiLayer->BeginFrame();

            for (auto layer : m_LayerStack)
                layer->OnImGuiRender();

            m_ImGuiLayer->EndFrame();

            Renderer::EndFrame();

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
