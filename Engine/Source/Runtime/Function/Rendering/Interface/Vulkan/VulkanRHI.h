#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/RHI.h"

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_NO_EXCEPTIONS 1
#define VULKAN_HPP_NO_CONSTRUCTORS

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

namespace Snowy::Ark
{
constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;
constexpr int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> g_ValidationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> g_DeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

//constexpr bool g_EnableValidationLayers = true;
#ifdef NDEBUG
constexpr bool g_EnableValidationLayers = false;
#else
constexpr bool g_EnableValidationLayers = true;
#endif

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool IsComplete() 
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};
struct SwapChainSupportDetails
{
    vk::SurfaceCapabilitiesKHR capabilities = {};
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

class VulkanRHI final : public RHI
{
public:
    VulkanRHI();
    void Run() override;

private:
    GLFWwindow* m_Window;
    vk::Instance m_Instance;
    vk::DebugUtilsMessengerEXT m_Callback;

    vk::PhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    vk::Device m_Device;

    vk::Queue m_GraphicsQueue;
    vk::Queue m_PresentQueue;

    vk::SurfaceKHR m_Surface;

    vk::SwapchainKHR m_SwapChain;
    vk::Format m_SwapChainImageFormat;
    vk::Extent2D m_SwapChainExtent;
    std::vector<vk::Image> m_SwapChainImages;
    std::vector<vk::ImageView> m_SwapChainImageViews;
    std::vector<vk::Framebuffer> m_SwapChainFramebuffers;

    vk::RenderPass m_RenderPass;
    vk::Pipeline m_GraphicsPipeline;
    vk::PipelineLayout m_PipelineLayout;

    vk::CommandPool m_CommandPool;
    std::vector<vk::CommandBuffer> m_CommandBuffers;

    std::vector<vk::Semaphore> m_ImageAvailableSemaphores;
    std::vector<vk::Semaphore> m_RenderFinishedSemaphores;
    std::vector<vk::Fence> m_InFlightFences;
    size_t m_CurrentFrame = 0;

private:
    void InitWindow();
    void InitVulkan();
    void MainLoop();
    void Cleanup();
    
    // ==============================================
    // Feature Functions
    // ==============================================
    void CreateInstance();

    void SetupDebugCallback();

    void CreateSurface();

    void PickPhysicalDevice();

    void CreateLogicalDevice();

    void CreateSwapChain();

    void CreateImageViews();

    void CreateGraphicsPipeline();
    
    void CreateRenderPass();

    void CreateFramebuffers();

    void CreateCommandPool();

    void CreateCommandBuffers();

    void CreateSyncObjects();

    void RecordCommandBuffer(std::vector<vk::CommandBuffer>& commandBuffers, uint32_t imageIndex);
    
    void DrawFrame();

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, 
                                                          const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        std::cerr << std::format("Validation layer: {}\n", pCallbackData->pMessage);
        return VK_FALSE;
    }

// ==============================================
// Tool Functions
// ==============================================
private:
    bool CheckValidationLayerSupport();

    bool CheckDeviceExtensionSupport(vk::PhysicalDevice device);

    std::vector<const char*> GetRequiredExtensions();

    bool IsDeviceSuitable(vk::PhysicalDevice device);

    QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice device);

    SwapChainSupportDetails QuerySwapChainSupport(vk::PhysicalDevice device);

    vk::SurfaceFormatKHR ChooseSwapChainFormat(std::span<vk::SurfaceFormatKHR> availableFormats);

    vk::PresentModeKHR ChooseSwapPresentMode(std::span<vk::PresentModeKHR> availablePresentModes);

    vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

    vk::ShaderModule CreateShaderModule(const std::vector<char>& code);

    static std::vector<char> ReadFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error(std::format("Failed to open file: {}", filename));
        }
        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }
};
}