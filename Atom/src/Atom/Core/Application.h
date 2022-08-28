#pragma once

#include "Events/Events.h"
#include "LayerStack.h"
#include "Timer.h"
#include "Window.h"

#include "Atom/ImGui/ImGuiLayer.h"

namespace Atom
{
    struct ApplicationSpecification
    {
        String    Name = "Atom Application";
        u32       WindowWidth = 1280;
        u32       WindowHeight = 720;
        bool      VSync = true;
    };

    class Application
    {
    public:
        Application(const ApplicationSpecification& spec);
        virtual ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        void Run();
        void OnEvent(Event& event);
        void Close();
        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);

        inline Window& GetWindow() { return *m_Window; }
        inline const ApplicationSpecification& GetSpecification() { return m_Specification; }
        inline ImGuiLayer& GetImGuiLayer() { return *m_ImGuiLayer; }
    private:
        bool OnWindowClosed(WindowClosedEvent& event);
    public:
        static inline Application& Get() { return *ms_Application; }
    private:
        ApplicationSpecification m_Specification;
        bool                     m_Running = true;
        Timer                    m_FrameTimer;
        Scope<Window>            m_Window;
        LayerStack               m_LayerStack;
        ImGuiLayer*              m_ImGuiLayer;
    private:
        static Application*      ms_Application;
    };

    Application* CreateApplication();
}

