﻿#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/RHIManager.h"

namespace Snowy::Ark
{
struct RenderSystemConfig
{
    RHIConfig rhi;
};

class RenderSystem
{
public:
    RenderSystem() = default;

    void Init(RenderSystemConfig config);
    void Tick();
    void Destory();

private:
    SharedHandle<RHI> m_RHIContext;
};
}

