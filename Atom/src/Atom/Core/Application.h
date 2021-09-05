#pragma once

#include "Events/Events.h"
#include "LayerStack.h"
#include "Timer.h"

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
        void Close();

        void OnEvent(Event& event);
    private:
        bool OnWindowClosed(WindowClosedEvent& event);
    public:
        static inline Application& GetApplicationInstance() { return *ms_Application; }
    private:
        bool                m_Running = true;
        LayerStack          m_LayerStack;
        Timer               m_FrameTimer;

    private:
        static Application* ms_Application;
    };

}

