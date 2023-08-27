#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Core/Log/LogSystem.h"
#include "Engine/Source/Runtime/Function/Global/GlobalContext.h"
#include "Engine/Source/Runtime/Function/Window/WindowSystem.h"
#include "Engine/Source/Runtime/Function/Rendering/RenderSystem.h"

#include <chrono>

namespace Snowy::Ark
{
class Engine
{
public:
    void Init(Ref<EngineConfig> config);
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


