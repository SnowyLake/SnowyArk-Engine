#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Global/GlobalTypedef.h"
#include "Engine/Source/Runtime/Function/Global/GlobalContext.h"
#include "Engine/Source/Runtime/Core/Log/LogSystem.h"

#define LOG_HELPER(level, fmt, ...)                                                          \
g_GlobalContext.logSys->Log(level, std::vformat("[{}] {}",                                   \
                            std::make_format_args(                                           \
                                std::string(__FUNCTION__),                              \
                                std::vformat(fmt, std::make_format_args(##__VA_ARGS__)) \
                            )));

#define LOG_DEBUG(fmt, ...)  LOG_HELPER(ELogLevel::Debug, fmt, ##__VA_ARGS__);

#define LOG_INFO(fmt, ...)   LOG_HELPER(ELogLevel::Info,  fmt, ##__VA_ARGS__);

#define LOG_WARN(fmt, ...)   LOG_HELPER(ELogLevel::Warn,  fmt, ##__VA_ARGS__);

#define LOG_ERROR(fmt, ...)  LOG_HELPER(ELogLevel::Error, fmt, ##__VA_ARGS__);

#define LOG_FATAL(fmt, ...)  LOG_HELPER(ELogLevel::Fatal, fmt, ##__VA_ARGS__);
