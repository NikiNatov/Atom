#include "atompch.h"
#include "Application.h"

#include "Core.h"
#include "Logger.h"

namespace Atom
{
    Application* Application::ms_Application = nullptr;

    Application::Application()
    {
        ATOM_ENGINE_ASSERT(ms_Application == nullptr, "Application already exists!");
        ms_Application = this;
    }

    Application::~Application()
    {
    }

    void Application::Run()
    {

        while (m_Running)
        {

        }
    }

    void Application::Close()
    {
        m_Running = false;
    }
}
