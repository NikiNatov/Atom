#pragma once

#include "AtomWin.h"
#include "Atom/Core/Window.h"

namespace Atom
{
#if defined(ATOM_PLATFORM_WINDOWS)

    class WindowWin32 : public Window
    {
    public:
        WindowWin32(const WindowProperties& properties);
        ~WindowWin32();

        virtual void ProcessEvents() override;
        virtual void SwapBuffers() override;
        virtual void SetEventCallback(const EventCallbackFn& callback) override;
        virtual void SetMinimized(bool state) override;
        virtual void ToggleVSync() override;

        virtual const String& GetTitle() const override { return m_Title; }
        virtual u32 GetWidth() const override { return m_Width; }
        virtual u32 GetHeight() const override { return m_Height; }
        virtual bool IsVSyncOn() const override { return m_VSync; }
        virtual bool IsMinimized() const override { return m_Minimized; }

        virtual u64 GetWindowHandle() const override { return (u64)m_WindowHandle; }
        virtual Device& GetDevice() const override { return *m_Device; };
        virtual const SwapChain& GetSwapChain() const override { return *m_SwapChain; };
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

        Ref<Device>     m_Device;
        Ref<SwapChain>  m_SwapChain;
    };

#endif // ATOM_PLATFORM_WINDOWS
}