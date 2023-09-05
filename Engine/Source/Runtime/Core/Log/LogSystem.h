#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Global/GlobalContextConfig.h"

#include <spdlog/spdlog.h>

namespace Snowy::Ark
{
class LogSystem
{
public:
    void Init(In<LogSystemConfig> config);
    void Destory();

    void Log(ELogLevel level, SStringIn msg);

private:
    AnsiString MessageConvert(SStringIn msg);
    void FatalCallback();

private:
    SharedHandle<spdlog::logger> m_Logger;
    ELogOutputTarget m_OutputTarget;
};
}
