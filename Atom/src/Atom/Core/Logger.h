#pragma once

#include <memory>
#include <spdlog\spdlog.h>
#include <spdlog\fmt\ostr.h>

namespace Atom
{
    class Logger
    {
    public:
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        static void Initialize();

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