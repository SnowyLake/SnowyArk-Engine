#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Global/GlobalTypedef.h"
#include "Engine/Source/Runtime/Function/Global/GlobalContext.h"
#include "Engine/Source/Runtime/Core/Log/LogSystem.h"

#if defined(SNOWY_CORE_CHAR_WIDE)
    #define SA_LOG_HELPER(level, fmt, ...)                                                   \
    g_GlobalContext.logSys->Log(level, std::vformat(STEXT("[{}] {}"),                        \
                                std::make_wformat_args(                                      \
                                    ANSI_TO_SSTR(std::string(__FUNCTION__)),                 \
                                    std::vformat(fmt, std::make_wformat_args(##__VA_ARGS__)) \
                                )));
#else
    #define SA_LOG_HELPER(level, fmt, ...)                                                   \
    g_GlobalContext.logSys->Log(level, std::vformat(STEXT("[{}] {}"),                        \
                                std::make_format_args(                                       \
                                    ANSI_TO_SSTR(std::string(__FUNCTION__)),                 \
                                    std::vformat(fmt, std::make_format_args(##__VA_ARGS__))  \
                                )));
#endif  // defined(SNOWY_CORE_CHAR_WIDE)


#define SA_LOG_DEBUG(fmt, ...)  SA_LOG_HELPER(ELogLevel::Debug, fmt, ##__VA_ARGS__);

#define SA_LOG_INFO(fmt, ...)   SA_LOG_HELPER(ELogLevel::Info,  fmt, ##__VA_ARGS__);

#define SA_LOG_WARN(fmt, ...)   SA_LOG_HELPER(ELogLevel::Warn,  fmt, ##__VA_ARGS__);

#define SA_LOG_ERROR(fmt, ...)  SA_LOG_HELPER(ELogLevel::Error, fmt, ##__VA_ARGS__);

#define SA_LOG_FATAL(fmt, ...)  SA_LOG_HELPER(ELogLevel::Fatal, fmt, ##__VA_ARGS__);
