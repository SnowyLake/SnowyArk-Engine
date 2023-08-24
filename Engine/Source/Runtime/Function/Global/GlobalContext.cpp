#include "GlobalContext.h"
namespace Snowy::Ark
{
RuntimeGlobalContext g_GlobalContext;

void RuntimeGlobalContext::Init(RuntimeGlobalContextConfig config)
{
    windowSys = MakeShared<WindowSystem>();

    renderSys = MakeShared<RenderSystem>();
    renderSys->Init(config.renderSys);
}
void RuntimeGlobalContext::Destory()
{
    renderSys->Destory();
}
}