#pragma once

#include <Atom/Core/EntryPoint.h>
#include "EditorLayer.h"

class EditorApplication : public Atom::Application
{
public:
    EditorApplication(const Atom::ApplicationSpecification& spec)
        : Application(spec)
    {
        Atom::ConsolePanel::Initialize();
        PushLayer(new Atom::EditorLayer());
    }

    ~EditorApplication()
    {
        Atom::ConsolePanel::Shutdown();
    }

};

Atom::Application* Atom::CreateApplication()
{
    Atom::ApplicationSpecification spec;
    spec.Name = "Atom Editor";
    return new EditorApplication(spec);
}