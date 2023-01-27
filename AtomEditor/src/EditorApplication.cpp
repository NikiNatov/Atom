#pragma once

#include <Atom/Core/EntryPoint.h>
#include "Panels/ConsolePanel.h"
#include "EditorLayer.h"

namespace Atom
{
    class EditorApplication : public Application
    {
    public:
        EditorApplication(const ApplicationSpecification& spec)
            : Application(spec)
        {
            PushLayer(new EditorLayer());
        }

        ~EditorApplication()
        {
        }

    };

    Application* CreateApplication(const CommandLineArgs& args)
    {
        ApplicationSpecification spec;
        spec.Name = "Atom Editor";
        spec.CommandLineArgs = args;
        spec.VSync = false;
        spec.AppLoggerSinks = {
            { std::make_shared<spdlog::sinks::stdout_color_sink_mt>(), "%^[%T] %n: %v%$" },
            { std::make_shared<ConsoleSink>(), "%^[%T] [%l]: %v%$" }
        };

        return new EditorApplication(spec);
    }
}
