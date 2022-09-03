#include "atompch.h"
#include "Logger.h"

#include "Atom/Tools/ConsoleSink.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Atom
{
    std::shared_ptr<spdlog::logger> Logger::ms_EngineLogger;
    std::shared_ptr<spdlog::logger> Logger::ms_ClientLogger;

    void Logger::Initialize()
    {
        Vector<spdlog::sink_ptr> engineSinks = {
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
        };

        Vector<spdlog::sink_ptr> clientSinks = {
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
            std::make_shared<ConsoleSink>()
        };

        engineSinks[0]->set_pattern("%^[%T] %n: %v%$");

        clientSinks[0]->set_pattern("%^[%T] %n: %v%$");
        clientSinks[1]->set_pattern("%^[%T] [%l]: %v%$");

        ms_EngineLogger = std::make_shared<spdlog::logger>("ATOM", engineSinks.begin(), engineSinks.end());
        ms_EngineLogger->set_level(spdlog::level::trace);

        ms_ClientLogger = std::make_shared<spdlog::logger>("CLIENT", clientSinks.begin(), clientSinks.end());
        ms_ClientLogger->set_level(spdlog::level::trace);
    }
}