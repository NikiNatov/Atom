#include "atompch.h"
#include "Logger.h"

namespace Atom
{
    std::shared_ptr<spdlog::logger> Logger::ms_EngineLogger;
    std::shared_ptr<spdlog::logger> Logger::ms_ClientLogger;

    void Logger::Initialize(const Vector<SinkWrapper>& clientLoggerSinks)
    {
        // Init engine sinks
        Vector<spdlog::sink_ptr> engineSinks = {
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
        };

        engineSinks[0]->set_pattern("%^[%T] %n: %v%$");

        // Init client sinks
        Vector<spdlog::sink_ptr> clientSinks;

        if (clientLoggerSinks.empty())
        {
            auto defaultSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            defaultSink->set_pattern("%^[%T] %n: %v%$");
            clientSinks.push_back(defaultSink);
        }
        else
        {
            clientSinks.reserve(clientLoggerSinks.size());

            for (const auto& sinkWrapper : clientLoggerSinks)
            {
                clientSinks.push_back(sinkWrapper.GetSink());
            }
        }

        //clientSinks[0]->set_pattern("%^[%T] %n: %v%$");
        //clientSinks[1]->set_pattern("%^[%T] [%l]: %v%$");

        ms_EngineLogger = std::make_shared<spdlog::logger>("ATOM", engineSinks.begin(), engineSinks.end());
        ms_EngineLogger->set_level(spdlog::level::trace);

        ms_ClientLogger = std::make_shared<spdlog::logger>("CLIENT", clientSinks.begin(), clientSinks.end());
        ms_ClientLogger->set_level(spdlog::level::trace);
    }
}