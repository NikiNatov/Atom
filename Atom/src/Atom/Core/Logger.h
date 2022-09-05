#pragma once

#include "Atom/Core/Core.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Atom
{
    class SinkWrapper
    {
    public:
        SinkWrapper(const spdlog::sink_ptr& sink, const std::string& pattern)
            : m_Sink(sink)
        {
            m_Sink->set_pattern(pattern.c_str());
        }

        spdlog::sink_ptr GetSink() const { return m_Sink; }
    private:
        spdlog::sink_ptr m_Sink;
    };

    class Logger
    {
    public:
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        static void Initialize(const std::vector<SinkWrapper>& clientSinks);

        static const std::shared_ptr<spdlog::logger>& GetEngineLogger() { return ms_EngineLogger; }
        static const std::shared_ptr<spdlog::logger>& GetClientLogger() { return ms_ClientLogger; }
    private:
        Logger() = default;
    private:
        static std::shared_ptr<spdlog::logger> ms_EngineLogger;
        static std::shared_ptr<spdlog::logger> ms_ClientLogger;
    };
}

#define ATOM_ENGINE_TRACE(...)		Atom::Logger::GetEngineLogger()->trace(__VA_ARGS__)
#define ATOM_ENGINE_INFO(...)		Atom::Logger::GetEngineLogger()->info(__VA_ARGS__)
#define ATOM_ENGINE_WARNING(...)	Atom::Logger::GetEngineLogger()->warn(__VA_ARGS__)
#define ATOM_ENGINE_ERROR(...)		Atom::Logger::GetEngineLogger()->error(__VA_ARGS__)
#define ATOM_ENGINE_CRITICAL(...)	Atom::Logger::GetEngineLogger()->critical(__VA_ARGS__)

#define ATOM_TRACE(...)				Atom::Logger::GetClientLogger()->trace(__VA_ARGS__)
#define ATOM_INFO(...)				Atom::Logger::GetClientLogger()->info(__VA_ARGS__)
#define ATOM_WARNING(...)			Atom::Logger::GetClientLogger()->warn(__VA_ARGS__)
#define ATOM_ERROR(...)				Atom::Logger::GetClientLogger()->error(__VA_ARGS__)
#define ATOM_CRITICAL(...)			Atom::Logger::GetClientLogger()->critical(__VA_ARGS__)