#pragma once

#include <Atom/Core/EntryPoint.h>
#include "RuntimeLayer.h"

class RuntimeApplication : public Atom::Application
{
public:
    RuntimeApplication(const Atom::ApplicationSpecification& spec)
        : Application(spec)
    {
        PushLayer(new Atom::RuntimeLayer());
    }

    ~RuntimeApplication()
    {
    }

};

Atom::Application* Atom::CreateApplication()
{
    Atom::ApplicationSpecification spec;
    return new RuntimeApplication(spec);
}