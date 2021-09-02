#pragma once

#include "Application.h"

extern Atom::Application* CreateApplication();

int main(int argc, char** argv)
{
    Atom::Application* app = CreateApplication();
    app->Run();

    delete app;
}