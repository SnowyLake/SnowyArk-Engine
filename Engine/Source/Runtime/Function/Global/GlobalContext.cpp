#include "GlobalContext.h"
#include "Engine/Source/Runtime/Core/Log/LogSystem.h"
#include "Engine/Source/Runtime/Function/Window/WindowSystem.h"
#include "Engine/Source/Runtime/Function/Rendering/RenderSystem.h"
namespace Snowy::Ark
{
RuntimeGlobalContext g_GlobalContext;

void RuntimeGlobalContext::Init(Ref<RuntimeGlobalContextConfig> config)
{
    logSys = MakeShared<LogSystem>();
    logSys->Init(config.logSys);

    auto& windowSysConfig = config.windowSys;
    windowSysConfig.rhiBackend = config.renderSys.rhi.backend;
    windowSys = MakeShared<WindowSystem>();
    windowSys->Init(windowSysConfig);

    auto& renderSysConfig = config.renderSys;
    renderSysConfig.rhi.windowHandle = windowSys->GetHandle();
    renderSys = MakeShared<RenderSystem>();
    renderSys->Init(renderSysConfig);
}
void RuntimeGlobalContext::Destory()
{
    renderSys->Destory();
    windowSys->Destory();
    logSys->Destory();
}
}
