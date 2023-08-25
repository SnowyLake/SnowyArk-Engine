#include "Engine/Source/Editor/Include/Editor.h"

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "SnowyArkRuntime.lib")

int main()
{
    namespace Ark = Snowy::Ark;
    try
    {
        Ark::Engine* engine = new Ark::Engine();
        Ark::EngineConfig engineConfig = {
            .runtimeGlobalContext = Ark::RuntimeGlobalContextConfig
            {
                .windowSys = Ark::WindowSystemConfig
                {
                    .width = 800,
                    .height = 600,
                    .title = "SnowyArk",
                    .isFullscreen = false,
                },
                .renderSys = Ark::RenderSystemConfig
                {
                    .rhi = Ark::RHIConfig
                    {
                        .backend = Ark::ERHIBackend::Vulkan,
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
    } catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}