#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Global/GlobalContextConfig.h"

#include <chrono>

namespace Snowy::Ark
{
struct EngineConfig
{
    RuntimeGlobalContextConfig runtimeGlobalContext;
};

class Engine
{
public:
    void Init(EngineConfig config);
    void Run();
    void Destroy();
private:
    void Tick(float deltaTime);
    void LogicTick(float deltaTime);
    void RenderingTick(float deltaTime);
    float CalculateDeltaTime();

    std::chrono::steady_clock::time_point m_LastTickTimePoint = std::chrono::steady_clock::now();
};
}


