#include "Engine/Source/Runtime/Engine.h"

namespace Snowy::Ark
{
void Engine::Init(EngineConfig config)
{
    g_GlobalContext.Init(config.runtimeGlobalContext);
}
void Engine::Run()
{
    SharedHandle<RenderSystem> renderSys = g_GlobalContext.renderSys;
    renderSys->Tick();
}
void Engine::Destroy()
{
    g_GlobalContext.Destory();
}
}