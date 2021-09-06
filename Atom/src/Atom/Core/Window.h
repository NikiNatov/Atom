#pragma once

#include "Core.h"
#include "Events/Events.h"

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
        virtual ~Window() = default;

        virtual void OnUpdate() = 0;
        virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
        virtual void SetMinimized(bool state) = 0;
        virtual void ToggleVSync() = 0;

        virtual const String& GetTitle() const = 0;
        virtual u32 GetWidth() const = 0;
        virtual u32 GetHeight() const = 0;
        virtual bool IsVSyncOn() const = 0;
        virtual bool IsMinimized() const = 0;

        virtual u64 GetWindowHandle() const = 0;

        static Scope<Window> Create(const WindowProperties& properties);
    };
}