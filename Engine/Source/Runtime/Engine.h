#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Global/GlobalContext.h"
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
};
}


