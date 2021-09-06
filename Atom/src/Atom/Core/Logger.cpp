#include "atompch.h"
#include "Logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Atom
{
    std::shared_ptr<spdlog::logger> Logger::ms_EngineLogger;
    std::shared_ptr<spdlog::logger> Logger::ms_ClientLogger;

    void Logger::Initialize()
    {
        spdlog::set_pattern("%^[%T] %n: %v%$");

        ms_EngineLogger = spdlog::stdout_color_mt("ATOM");
        ms_ClientLogger = spdlog::stdout_color_mt("CLIENT");

        ms_EngineLogger->set_level(spdlog::level::trace);
        ms_ClientLogger->set_level(spdlog::level::trace);
    }
}