#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Global/GlobalContextConfig.h"

#include "Engine/Source/Runtime/Function/Rendering/Interface/RHI.h"

namespace Snowy::Ark
{
class RenderSystem
{
public:
    RenderSystem() = default;

    void Init(Ref<RenderSystemConfig> config);
    void Tick();
    void Destory();

private:
    SharedHandle<RHI> m_RHIContext;
};
}
