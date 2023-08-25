#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Global/GlobalContextConfig.h"
namespace Snowy::Ark
{
class WindowSystem;
class RenderSystem;

struct RuntimeGlobalContext
{
    void Init(RuntimeGlobalContextConfig config);
    void Destory();

    SharedHandle<WindowSystem> windowSys;
    SharedHandle<RenderSystem> renderSys;
};

extern RuntimeGlobalContext g_GlobalContext;
}