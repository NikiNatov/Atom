#pragma once

#include <Atom/Core/EntryPoint.h>
#include "Panels/ConsolePanel.h"
#include "EditorLayer.h"

class EditorApplication : public Atom::Application
{
public:
    EditorApplication(const Atom::ApplicationSpecification& spec)
        : Application(spec)
    {
        PushLayer(new Atom::EditorLayer());
    }

    ~EditorApplication()
    {
    }

};

Atom::Application* Atom::CreateApplication()
{
    Atom::ApplicationSpecification spec;
    spec.Name = "Atom Editor";
    spec.AppLoggerSinks = {
        { std::make_shared<spdlog::sinks::stdout_color_sink_mt>(), "%^[%T] %n: %v%$" },
        { std::make_shared<ConsoleSink>(), "%^[%T] [%l]: %v%$" }
    };

    return new EditorApplication(spec);
}