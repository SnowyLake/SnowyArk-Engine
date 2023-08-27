#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Global/GlobalContextConfig.h"
namespace Snowy::Ark
{
class WindowSystem;
class RenderSystem;
class LogSystem;

struct RuntimeGlobalContext
{
    void Init(Ref<RuntimeGlobalContextConfig> config);
    void Destory();

    SharedHandle<WindowSystem> windowSys;
    SharedHandle<RenderSystem> renderSys;
    SharedHandle<LogSystem> logSys;
};

extern RuntimeGlobalContext g_GlobalContext;
}