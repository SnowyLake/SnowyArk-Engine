#include "Engine/Source/Runtime/Engine.h"
#include "Engine/Source/Runtime/Core/Log/Logger.h"
#include "Engine/Source/Runtime/Function/Global/GlobalContext.h"
#include "Engine/Source/Runtime/Function/Window/WindowSystem.h"
#include "Engine/Source/Runtime/Function/Rendering/RenderSystem.h"
namespace Snowy::Ark
{
void Engine::Init(Ref<EngineConfig> config)
{
    g_GlobalContext.Init(config.runtimeGlobalContext);
}

void Engine::Run()
{
    SharedHandle windowSys = g_GlobalContext.windowSys;
    ASSERT(windowSys);
    while (!windowSys->ShouldClose())
    {
        const float deltaTime = CalculateDeltaTime();
        Tick(deltaTime);
    }
}

void Engine::Destroy()
{
    g_GlobalContext.Destory();
}

void Engine::Tick(float deltaTime)
{
    LogicTick(deltaTime);

    RenderingTick(deltaTime);
    g_GlobalContext.windowSys->PollEvents();
}
void Engine::LogicTick(float deltaTime)
{
}

void Engine::RenderingTick(float deltaTime)
{
    g_GlobalContext.renderSys->Tick();
}

float Engine::CalculateDeltaTime()
{
    float deltaTime;
    {
        using namespace std::chrono;

        steady_clock::time_point tickTimePoint = steady_clock::now();
        duration<float> timeSpan = duration_cast<duration<float>>(tickTimePoint - m_LastTickTimePoint);
        deltaTime = timeSpan.count();

        m_LastTickTimePoint = tickTimePoint;
    }
    return deltaTime;
}
}