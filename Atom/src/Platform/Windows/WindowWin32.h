#pragma once

#include "AtomWin.h"
#include "Atom/Core/Window.h"

namespace Atom
{
    class WindowWin32 : public Window
    {
    public:
        WindowWin32(const WindowProperties& properties);
        ~WindowWin32();

        virtual void OnUpdate() override;
        virtual void SetEventCallback(const EventCallbackFn& callback) override;
        virtual void SetMinimized(bool state) override;
        virtual void ToggleVSync() override;

        virtual const String& GetTitle() const override { return m_Title; }
        virtual u32 GetWidth() const override { return m_Width; }
        virtual u32 GetHeight() const override { return m_Height; }
        virtual bool IsVSyncOn() const override { return m_VSync; }
        virtual bool IsMinimized() const override { return m_Minimized; }

        virtual u64 GetWindowHandle() const override { return (u64)m_WindowHandle; }
    private:
        static LRESULT WINAPI WindowProcSetup(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
        static LRESULT WINAPI WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    private:
        String          m_Title;
        u32             m_Width;
        u32             m_Height;
        bool            m_VSync;
        bool            m_Minimized;

        HWND            m_WindowHandle;
        WNDCLASSEX      m_WindowClass;
        RECT            m_WindowRect;
        EventCallbackFn m_EventCallback;
    };
}