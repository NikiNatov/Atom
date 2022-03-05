#pragma once

#include <Atom.h>

class SandboxApplication : public Atom::Application
{
public:
    SandboxApplication(const Atom::ApplicationSpecification& spec)
        : Application(spec)
    {
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