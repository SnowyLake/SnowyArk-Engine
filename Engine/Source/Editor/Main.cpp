#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanRHI.h"

#pragma comment(lib, "vulkan-1.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "SnowyArkRuntime.lib")
int main()
{
    try
    {
        auto app = std::make_unique<Ark::VulkanRHI>();
        app->Run();
    } catch(const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS; 
}