#include "atompch.h"
#include "Application.h"

#include "Core.h"
#include "Logger.h"
#include "Atom/Renderer/Adapter.h"
#include "Atom/Renderer/Device.h"
#include "Atom/Renderer/CommandQueue.h"
#include "Atom/Renderer/GraphicsCommandList.h"
#include "Atom/Renderer/Renderer.h"

#include "Platform/DirectX12/DX12Adapter.h"
#include "Platform/DirectX12/DX12Device.h"
#include "Platform/DirectX12/DX12CommandQueue.h"
#include "Platform/DirectX12/DX12GraphicsCommandList.h"

namespace Atom
{
    Application* Application::ms_Application = nullptr;

    // -----------------------------------------------------------------------------------------------------------------------------
    Application::Application()
    {
        ATOM_ENGINE_ASSERT(ms_Application == nullptr, "Application already exists!");
        ms_Application = this;

        WindowProperties properties;
        properties.EventCallback = ATOM_BIND_EVENT_FN(Application::OnEvent);

        m_Window = Window::Create(properties);

        Renderer::Initialize();

        
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Application::~Application()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Application::Run()
    {
        while (m_Running)
        {
            Timestep ts = m_FrameTimer.GetElapsedTime();
            //ATOM_ENGINE_INFO("{0}ms", ts.GetMilliseconds());

            m_FrameTimer.Reset();

            if (!m_Window->IsMinimized())
            {
                for (auto layer : m_LayerStack)
                    layer->OnUpdate(ts);
            }
            m_Window->OnUpdate();

            Renderer::BeginFrame();

            Renderer::EndFrame();

            m_FrameTimer.Stop();
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Application::Close()
    {
        m_Running = false;
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
