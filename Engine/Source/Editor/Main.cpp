#include "Engine/Source/Editor/Include/Editor.h"

#pragma comment(lib, "SnowyArkRuntime.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "spdlog.lib")

int main()
{
    namespace Ark = Snowy::Ark;

    Ark::Engine* engine = new Ark::Engine();
    Ark::EngineConfig engineConfig = {
        .runtimeGlobalContext = Ark::RuntimeGlobalContextConfig
        {
            .windowSys = Ark::WindowSystemConfig
            {
                .width = 800,
                .height = 600,
                .title = STEXT("SnowyArk"),
                .isFullscreen = false,
            },
            .renderSys = Ark::RenderSystemConfig
            {
                .rhi = Ark::RHIConfig
                {
                    .backend = Ark::ERHIBackend::Vulkan,
                    .maxFrameInFlight = 2,
                    .vkEnableValidationLayers = true,
                    .vkValidationLayers = { "VK_LAYER_KHRONOS_validation" },
                    .vkDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME },
                },
            },
        },
    };
    engine->Init(engineConfig);

    Ark::Editor* editor = new Ark::Editor();
    editor->Init(engine);

    editor->Run();

    editor->Destroy();
    engine->Destroy();
}