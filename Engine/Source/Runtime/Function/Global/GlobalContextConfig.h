#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Global/GlobalTypedef.h"

struct GLFWwindow;

namespace Snowy::Ark
{
struct WindowSystemConfig
{
    uint32_t        width        = 1280;
    uint32_t        height       = 720;
    const AnsiChar* title        = "SnowyArk";
    bool            isFullscreen = false;
};

// RHI context config
struct RHIConfig
{
    ERHIBackend           backend      = ERHIBackend::Vulkan;
    RawHandle<GLFWwindow> windowHandle = nullptr;
};

// Render system config
struct RenderSystemConfig
{
    RHIConfig rhi;
};

// Runtime global context config
struct RuntimeGlobalContextConfig
{
    WindowSystemConfig windowSys;
    RenderSystemConfig renderSys;
};
}