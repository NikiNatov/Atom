#pragma once

#include <Atom/Core/EntryPoint.h>
#include "SandboxLayer.h"

class SandboxApplication : public Atom::Application
{
public:
    SandboxApplication(const Atom::ApplicationSpecification& spec)
        : Application(spec)
    {
        PushLayer(new Atom::SandboxLayer());
    }

    ~SandboxApplication()
    {
    }

};

Atom::Application* Atom::CreateApplication()
{
    Atom::ApplicationSpecification spec;
    return new SandboxApplication(spec);
}