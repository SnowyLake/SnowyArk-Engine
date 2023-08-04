#include "Engine/Source/Editor/Include/Editor.h"

#pragma comment(lib, "vulkan-1.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "SnowyArkRuntime.lib")
int main()
{
    namespace Ark = Snowy::Ark;
    
    try
    {
        Ark::Engine* engine = new Ark::Engine();
        engine->Init(Ark::ERHIBackend::Vulkan);

        Ark::Editor* editor = new Ark::Editor();
        editor->Init(engine);

        editor->Run();

        editor->Destroy();
        engine->Destroy();

    } catch(const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS; 
}