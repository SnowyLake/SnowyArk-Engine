#include "Engine/Source/Runtime/Core/Log/LogSystem.h"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace Snowy::Ark
{
void LogSystem::Init(In<LogSystemConfig> config)
{
    m_OutputTarget = config.outputTarget;

    auto consoleSink = MakeShared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_level(spdlog::level::trace);
    consoleSink->set_pattern("[%^%l%$] %v");

    const spdlog::sinks_init_list sinkList = { consoleSink };

    spdlog::init_thread_pool(8192, 1);

    m_Logger = MakeShared<spdlog::async_logger>("muggle_logger", sinkList, spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    m_Logger->set_level(spdlog::level::trace);

    spdlog::register_logger(m_Logger);
}

void LogSystem::Destory()
{
    m_Logger->flush();
    spdlog::drop_all();
}

void LogSystem::Log(ELogLevel level, SStringIn msg)
{
    switch (level)
    {
    case ELogLevel::Debug:
        m_Logger->debug(MessageConvert(msg));
        break;
    case ELogLevel::Info:
        m_Logger->info(MessageConvert(msg));
        break;
    case ELogLevel::Warn:
        m_Logger->warn(MessageConvert(msg));
        break;
    case ELogLevel::Error:
        m_Logger->error(MessageConvert(msg));
        break;
    case ELogLevel::Fatal:
        m_Logger->critical(MessageConvert(msg));
        FatalCallback();
        break;
    default:
        break;
    }
}

AnsiString LogSystem::MessageConvert(SStringIn msg)
{
    if (m_OutputTarget == ELogOutputTarget::Console)
    {
        return SSTR_TO_ANSI(msg);
    }
    else
    {
        return SSTR_TO_UTF8(msg);
    }
}

void LogSystem::FatalCallback()
{
    std::terminate();
}
}
