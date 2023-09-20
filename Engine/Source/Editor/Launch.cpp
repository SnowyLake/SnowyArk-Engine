#include "Engine/Source/Editor/Include/Editor.h"

#pragma comment(lib, "SnowyArkRuntime.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "spdlog.lib")

int main()
{
    namespace Ark = Snowy::Ark;

    // TODO: Serialize to Json file
    Ark::EngineConfig engineConfig = {
        .runtimeGlobalContext = Ark::RuntimeGlobalContextConfig
        {
            .logSys = Ark::LogSystemConfig
            {
                .outputTarget = Ark::ELogOutputTarget::Console,
            },
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
                    .frameCountInFlight = 2,
                    .vkEnableValidationLayers = true,
                    .vkValidationLayers = { "VK_LAYER_KHRONOS_validation" },
                    .vkDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME },
                },
            },
        },
    };
    auto engine = Snowy::MakeUnique<Ark::Engine>();
    engine->Init(engineConfig);
    auto editor = Snowy::MakeUnique<Ark::Editor>();
    editor->Init(std::move(engine));
    editor->Run();
    editor->Destroy();
}
