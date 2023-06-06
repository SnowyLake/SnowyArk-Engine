#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanRHI.h"

int main()
{
    try
    {
        auto app = std::make_unique<Ark::Application>();
        app->Run();
    } catch(const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}