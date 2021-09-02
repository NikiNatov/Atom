#pragma once

#include <Atom.h>

class SandboxApplication : public Atom::Application
{
public:
    SandboxApplication()
    {
    }

    ~SandboxApplication()
    {
    }

};

Atom::Application* CreateApplication()
{
    return new SandboxApplication();
}