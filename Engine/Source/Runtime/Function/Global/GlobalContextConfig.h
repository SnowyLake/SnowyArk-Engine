#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Global/GlobalTypedef.h"

#include <vector>
struct GLFWwindow;

namespace Snowy::Ark
{
// WindowSystem Config
struct WindowSystemConfig
{
    uint32_t    width        = 1280;
    uint32_t    height       = 720;
    SString     title        = STEXT("SnowyArk");
    bool        isFullscreen = false;
    ERHIBackend rhiBackend   = ERHIBackend::Vulkan;
};

// RHI Context Config
struct RHIConfig
{
    ERHIBackend           backend          = ERHIBackend::Vulkan;
    RawHandle<GLFWwindow> windowHandle     = nullptr;
    uint32_t              maxFrameInFlight = 2;

    // Vulkan Context Config
    bool vkEnableValidationLayers = true;
    std::vector<const AnsiChar*> vkValidationLayers;
    std::vector<const AnsiChar*> vkDeviceExtensions;
};

// RenderSystem Config
struct RenderSystemConfig
{
    RHIConfig rhi;
};

// RuntimeGlobalContext Config
struct RuntimeGlobalContextConfig
{
    WindowSystemConfig windowSys;
    RenderSystemConfig renderSys;
};

struct EngineConfig
{
    RuntimeGlobalContextConfig runtimeGlobalContext;
};
}