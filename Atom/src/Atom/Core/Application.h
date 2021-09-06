#pragma once

#include "Events/Events.h"
#include "LayerStack.h"
#include "Timer.h"
#include "Window.h"

namespace Atom
{
    class Application
    {
    public:
        Application();
        virtual ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        void Run();
        void OnEvent(Event& event);
        void Close();

        inline Window& GetWindow() { return *m_Window; }
    private:
        bool OnWindowClosed(WindowClosedEvent& event);
    public:
        static inline Application& GetApplicationInstance() { return *ms_Application; }
    private:
        bool                m_Running = true;
        LayerStack          m_LayerStack;
        Timer               m_FrameTimer;
        Scope<Window>       m_Window;
    private:
        static Application* ms_Application;
    };

}

