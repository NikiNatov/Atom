#pragma once

#include "Application.h"
#include "Logger.h"

int main(int argc, char** argv)
{
#if defined (ATOM_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    Atom::Logger::Initialize();
    Atom::Application* app = Atom::CreateApplication();
    ATOM_ENGINE_INFO("Application created!");

    app->Run();

    delete app;
}