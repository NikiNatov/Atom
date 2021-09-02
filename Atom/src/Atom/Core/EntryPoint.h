#pragma once

#include "Application.h"
#include "Logger.h"

extern Atom::Application* CreateApplication();

int main(int argc, char** argv)
{
    Atom::Logger::Initialize();
    Atom::Application* app = CreateApplication();
    ATOM_ENGINE_INFO("Application created!");

    app->Run();

    delete app;
}