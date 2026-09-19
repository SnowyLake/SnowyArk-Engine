#include <Runtime/Core/Log.h>

#include <iostream>
#include <stdexcept>
#include <string>

#ifdef _WIN32
// WIN32_LEAN_AND_MEAN and NOMINMAX must be defined before windows.h.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace SnowyArk
{

const char* Log::LevelPrefix(const LogLevel level)
{
    switch (level)
    {
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warning:
        return "WARN";
    case LogLevel::Error:
        return "ERROR";
    }
    return "LOG";
}

void Log::Write(const LogLevel level, const std::string_view message)
{
    const std::string line = std::string("[") + LevelPrefix(level) + "] " + std::string(message) + "\n";
    std::ostream& stream = (level == LogLevel::Info) ? std::cout : std::cerr;
    stream << line << std::flush;

#ifdef _WIN32
    OutputDebugStringA(line.c_str());
#endif
}

void Log::Info(const std::string_view message)
{
    Write(LogLevel::Info, message);
}

void Log::Warning(const std::string_view message)
{
    Write(LogLevel::Warning, message);
}

void Log::Error(const std::string_view message)
{
    Write(LogLevel::Error, message);
}

void Log::Fatal(const std::string_view message)
{
    Write(LogLevel::Error, message);
    throw std::runtime_error(std::string(message));
}

}
