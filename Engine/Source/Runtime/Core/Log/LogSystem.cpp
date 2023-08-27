#include "Engine/Source/Runtime/Core/Log/LogSystem.h"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace Snowy::Ark
{
void LogSystem::Init()
{
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_level(spdlog::level::trace);
    consoleSink->set_pattern("[%^%l%$] %v");

    const spdlog::sinks_init_list sinkList = { consoleSink };

    spdlog::init_thread_pool(8192, 1);

    m_Logger = std::make_shared<spdlog::async_logger>("muggle_logger", sinkList.begin(), sinkList.end(),
                                                      spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    m_Logger->set_level(spdlog::level::trace);

    spdlog::register_logger(m_Logger);
}

void LogSystem::Destory()
{
    m_Logger->flush();
    spdlog::drop_all();
}
}