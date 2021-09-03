#pragma once

#include "Events\Events.h"

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

    public:
        static inline Application& GetApplicationInstance() { return *ms_Application; }
    private:
        bool m_Running = true;
    private:
        static Application* ms_Application;
    };

}

