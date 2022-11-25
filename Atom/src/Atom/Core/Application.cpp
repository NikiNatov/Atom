#include "atompch.h"
#include "Application.h"

#include "Atom/Core/Core.h"
#include "Atom/Core/Logger.h"
#include "Atom/Core/Input.h"
#include "Atom/Renderer/Renderer.h"
#include "Atom/Scripting/ScriptEngine.h"

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
        ScriptEngine::Initialize();
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

        Renderer::Shutdown();
        ScriptEngine::Shutdown();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Application::Run()
    {
        while (m_Running)
        {
            Timestep ts = m_FrameTimer.GetElapsedTime();

            m_FrameTimer.Reset();

            m_Window->ProcessEvents();

            Renderer::BeginFrame();

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
    bool Application::OnWindowClosed(WindowClosedEvent& event)
    {
        Close();
        return true;
    }
}
