#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Texture.h"

namespace Atom
{
    class ConsoleMessage
    {
    public:
        enum class Severity
        {
            None = 0,
            Info,
            Warning,
            Error
        };
    public:
        ConsoleMessage()
            : m_Severity(Severity::None) 
        {}

        ConsoleMessage(const String& message, Severity severity)
            : m_Message(message), m_Severity(severity)
        {}

        const String& GetMessageString() const { return m_Message; }
        Severity GetSeverity() const { return m_Severity; }
    private:
        String   m_Message;
        Severity m_Severity;
    };

    class ConsolePanel
    {
    public:
        static void Initialize();
        static void Shutdown();
        static void AddMessage(const ConsoleMessage& message);
        static void OnImGuiRender();
    private:
        static Vector<ConsoleMessage> ms_Messages;
        inline static Ref<Texture2D> ms_InfoIcon = nullptr;
        inline static Ref<Texture2D> ms_WarningIcon = nullptr;
        inline static Ref<Texture2D> ms_ErrorIcon = nullptr;
    };
}