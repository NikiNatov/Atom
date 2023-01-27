#pragma once

#include <Atom/Core/EntryPoint.h>
#include "RuntimeLayer.h"

namespace Atom
{
    class RuntimeApplication : public Application
    {
    public:
        RuntimeApplication(const ApplicationSpecification& spec)
            : Application(spec)
        {
            PushLayer(new RuntimeLayer());
        }

        ~RuntimeApplication()
        {
        }

    };

    Application* CreateApplication(const CommandLineArgs& args)
    {
        ApplicationSpecification spec;
        spec.Name = "Atom Runtime";
        spec.CommandLineArgs = args;
        return new RuntimeApplication(spec);
    }
}
