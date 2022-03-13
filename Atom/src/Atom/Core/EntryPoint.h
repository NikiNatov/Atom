#pragma once

#include "Application.h"
#include "Logger.h"

#if defined (ATOM_DEBUG) && defined (ATOM_PLATFORM_WINDOWS)
#include "Atom/Platform/DirectX12/DirectX12.h"
#endif

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

#if defined (ATOM_DEBUG) && defined (ATOM_PLATFORM_WINDOWS)
    wrl::ComPtr<IDXGIDebug1> dxgiDebug;
    DXCall(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)));
    DXCall(dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL)));
#endif

}