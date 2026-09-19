#pragma once

#include <format>
#include <string>
#include <string_view>
#include <utility>

namespace SnowyArk
{

enum class LogLevel
{
    Info,
    Warning,
    Error
};

class Log
{
public:
    /// Writes a log line at the given severity.
    static void Write(LogLevel level, std::string_view message);

    /// Writes an informational log line.
    static void Info(std::string_view message);

    /// Writes a warning log line.
    static void Warning(std::string_view message);

    /// Writes an error log line.
    static void Error(std::string_view message);

    /// Writes a fatal error log line and throws std::runtime_error.
    [[noreturn]] static void Fatal(std::string_view message);

    /// Formats and writes an informational log line.
    template <typename... Args> static void Info(const std::format_string<Args...> fmt, Args&&... args)
    {
        Info(std::format(fmt, std::forward<Args>(args)...));
    }

    /// Formats and writes a warning log line.
    template <typename... Args> static void Warning(const std::format_string<Args...> fmt, Args&&... args)
    {
        Warning(std::format(fmt, std::forward<Args>(args)...));
    }

    /// Formats and writes an error log line.
    template <typename... Args> static void Error(const std::format_string<Args...> fmt, Args&&... args)
    {
        Error(std::format(fmt, std::forward<Args>(args)...));
    }

    /// Formats a fatal error log line and throws std::runtime_error.
    template <typename... Args> [[noreturn]] static void Fatal(const std::format_string<Args...> fmt, Args&&... args)
    {
        Fatal(std::string_view { std::format(fmt, std::forward<Args>(args)...) });
    }

private:
    /// Returns a short ASCII tag for the given log severity.
    static const char* LevelPrefix(LogLevel level);
};

}
