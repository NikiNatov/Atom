#pragma once

#include "Core.h"
#include "Events/Events.h"

#include "Atom/Renderer/SwapChain.h"

namespace Atom
{
    using EventCallbackFn = std::function<void(Event&)>;

    struct WindowProperties
    {
        String          Title = "Atom Engine";
        u32             Width = 1280;
        u32             Height = 720;
        bool            VSync = true;
        EventCallbackFn EventCallback;

        WindowProperties()
        {}

        WindowProperties(const String& title, u32 width, u32 height, bool vsync, const EventCallbackFn& eventCallback)
            : Title(title), Width(width), Height(height), VSync(vsync), EventCallback(eventCallback)
        {}
    };

    class Window
    {
    public:
        Window(const WindowProperties& properties);
        ~Window();

        void ProcessEvents();
        void SwapBuffers();
        void SetEventCallback(const EventCallbackFn& callback);
        void SetMinimized(bool state);
        void ToggleVSync();

        const String& GetTitle() const { return m_Title; }
        u32 GetWidth() const { return m_Width; }
        u32 GetHeight() const { return m_Height; }
        bool IsVSyncOn() const { return m_VSync; }
        bool IsMinimized() const { return m_Minimized; }
        HWND GetWindowHandle() const { return m_WindowHandle; }
        const SwapChain* GetSwapChain() const { return m_SwapChain.get(); };
    private:
        static LRESULT WINAPI WindowProcSetup(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
        static LRESULT WINAPI WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    private:
        String          m_Title;
        u32             m_Width;
        u32             m_Height;
        bool            m_VSync;
        bool            m_Minimized;
        bool            m_NeedsResize;

        HWND            m_WindowHandle;
        WNDCLASSEX      m_WindowClass;
        RECT            m_WindowRect;
        EventCallbackFn m_EventCallback;

        Ref<SwapChain>  m_SwapChain;
    };
}