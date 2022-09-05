#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Texture.h"

#include <spdlog/sinks/base_sink.h>
#include <mutex>

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
        inline static Ref<Texture2D>  ms_InfoIcon = nullptr;
        inline static Ref<Texture2D>  ms_WarningIcon = nullptr;
        inline static Ref<Texture2D>  ms_ErrorIcon = nullptr;
    };

    class ConsoleSink : public spdlog::sinks::base_sink<std::mutex>
    {
    public:
        ConsoleSink() = default;
        ConsoleSink(const ConsoleSink&) = delete;
        ConsoleSink& operator=(const ConsoleSink&) = delete;

    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override
        {
            fmt::memory_buffer buffer;
            spdlog::sinks::base_sink<std::mutex>::formatter_->format(msg, buffer);

            m_CurrentMessage = ConsoleMessage(fmt::to_string(buffer), GetMessageSeverity(msg.level));
            flush_();
        }

        void flush_() override
        {
            if (m_CurrentMessage.GetSeverity() != ConsoleMessage::Severity::None)
                ConsolePanel::AddMessage(m_CurrentMessage);
        }
    private:
        ConsoleMessage::Severity GetMessageSeverity(spdlog::level::level_enum level)
        {
            switch (level)
            {
            case spdlog::level::trace:
            case spdlog::level::debug:
            case spdlog::level::info:
                return ConsoleMessage::Severity::Info;
            case spdlog::level::warn:
                return ConsoleMessage::Severity::Warning;
            case spdlog::level::err:
            case spdlog::level::critical:
                return ConsoleMessage::Severity::Error;
            }

            ATOM_ENGINE_ASSERT(false, "Unknown severity level");
            return ConsoleMessage::Severity::None;
        }
    private:
        ConsoleMessage m_CurrentMessage;
    };
}