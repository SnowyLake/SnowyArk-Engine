#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Global/GlobalContextConfig.h"

#include <spdlog/spdlog.h>

namespace Snowy::Ark
{
class LogSystem
{
public:
    void Init();
    void Destory();

    template<typename... Args>
    void Log(ELogLevel level, Args&&... args)
    {
        switch (level)
        {
            case ELogLevel::Debug:
                m_Logger->debug(std::forward<Args>(args)...);
                break;
            case ELogLevel::Info:
                m_Logger->info(std::forward<Args>(args)...);
                break;
            case ELogLevel::Warn:
                m_Logger->warn(std::forward<Args>(args)...);
                break;
            case ELogLevel::Error:
                m_Logger->error(std::forward<Args>(args)...);
                break;
            case ELogLevel::Fatal:
                m_Logger->critical(std::forward<Args>(args)...);
                FatalCallback(std::forward<Args>(args)...);
                break;
            default:
                break;
        }
    }

    template<typename... Args>
    void FatalCallback(Args&&... args)
    {

    }

private:
    std::shared_ptr<spdlog::logger> m_Logger;
};
}

