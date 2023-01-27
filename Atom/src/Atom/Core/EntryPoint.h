#pragma once

#include "Application.h"
#include "AtomWin.h"

#if 0
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
#if defined (ATOM_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    Atom::Application* app = Atom::CreateApplication({ argc, argv });
    ATOM_ENGINE_INFO("Application created!");

    app->Run();

    delete app;

    return 0;
}
#endif

int main(int argc, char** argv)
{
#if defined (ATOM_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    Atom::Application* app = Atom::CreateApplication({ argc, argv });
    ATOM_ENGINE_INFO("Application created!");

    app->Run();

    delete app;

    return 0;
}