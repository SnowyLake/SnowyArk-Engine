#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Global/GlobalContextConfig.h"
namespace Snowy::Ark
{
class WindowSystem;
class RenderSystem;
class LogSystem;
class AssetManager;

struct RuntimeGlobalContext
{
    void Init(Ref<RuntimeGlobalContextConfig> config);
    void Destory();

    SharedHandle<LogSystem> logSys;
    SharedHandle<AssetManager> assetMgr;
    SharedHandle<WindowSystem> windowSys;
    SharedHandle<RenderSystem> renderSys;
};

extern RuntimeGlobalContext g_RuntimeContext;
}
