#include "GlobalContext.h"
#include "Engine/Source/Runtime/Function/Window/WindowSystem.h"
#include "Engine/Source/Runtime/Function/Rendering/RenderSystem.h"
namespace Snowy::Ark
{
RuntimeGlobalContext g_GlobalContext;

void RuntimeGlobalContext::Init(RuntimeGlobalContextConfig config)
{
    windowSys = MakeShared<WindowSystem>();
    windowSys->Init(config.windowSys, config.renderSys);

    renderSys = MakeShared<RenderSystem>();
    renderSys->Init(config.renderSys, windowSys->GetHandle());
}
void RuntimeGlobalContext::Destory()
{
    renderSys->Destory();
}
}