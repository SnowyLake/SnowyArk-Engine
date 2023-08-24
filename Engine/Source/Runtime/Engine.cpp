#include "Engine/Source/Runtime/Engine.h"

namespace Snowy::Ark
{
void Engine::Init(EngineConfig config)
{
    g_RuntimeGlobalContext.Init(config.runtimeGlobalContext);
}
void Engine::Run()
{
    SharedHandle<RenderSystem> renderSys = g_RuntimeGlobalContext.renderSys;
    renderSys->Tick();
}
void Engine::Destroy()
{
    g_RuntimeGlobalContext.Destory();
}
}