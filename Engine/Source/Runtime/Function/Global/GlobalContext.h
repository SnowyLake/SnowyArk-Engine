#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Window/WindowSystem.h"
#include "Engine/Source/Runtime/Function/Rendering/RenderSystem.h"
namespace Snowy::Ark
{
struct RuntimeGlobalContextConfig
{
    RenderSystemConfig renderSys;
};
struct RuntimeGlobalContext
{
    void Init(RuntimeGlobalContextConfig config);
    void Destory();

    SharedHandle<WindowSystem> windowSys;
    SharedHandle<RenderSystem> renderSys;
};

extern RuntimeGlobalContext g_GlobalContext;
}


