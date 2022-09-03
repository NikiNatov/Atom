#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Tools/ConsolePanel.h"

#include <spdlog/sinks/base_sink.h>
#include <mutex>

namespace Atom
{
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
            if(m_CurrentMessage.GetSeverity() != ConsoleMessage::Severity::None)
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