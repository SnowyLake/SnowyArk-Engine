#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/RHI.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanUtils.h"

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

namespace Snowy::Ark
{
struct Vertex
{
    glm::vec2 position;
    glm::vec3 color;

    static vk::VertexInputBindingDescription GetBindingDescription() 
    {
        vk::VertexInputBindingDescription bindingDescription = {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = vk::VertexInputRate::eVertex,
        };
        return bindingDescription;
    }
    static std::array<vk::VertexInputAttributeDescription, 2> GetAttributeDescriptions()
    {
        std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};
        attributeDescriptions[0] = {
            .location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32Sfloat,
            .offset = offsetof(Vertex, position),
        };
        attributeDescriptions[1] = {
            .location = 1,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(Vertex, color),
        };
        return attributeDescriptions;
    }
};

const std::vector<Vertex> g_TriangleVertices = {
    { { 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f} },
    { { 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f} },
    { {-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f} }
};

constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;
constexpr int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> g_ValidationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> g_DeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

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
    void Cleanup() override;

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
    bool m_FramebufferResized = false;

    vk::Buffer m_VertexBuffer;
    vk::DeviceMemory m_VertexBufferMemory;

private:
    void InitWindow();
    void InitVulkan();
    void MainLoop();

    
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
    void CreateVertexBuffer();
    void CreateSyncObjects();
    void ReCreateSwapChain();
    void CleanupSwapChain();
    void RecordCommandBuffer(std::vector<vk::CommandBuffer>& commandBuffers, uint32_t imageIndex);
    void DrawFrame();

    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto rhi = reinterpret_cast<VulkanRHI*>(glfwGetWindowUserPointer(window));
        rhi->m_FramebufferResized = true;
    }

// ==============================================
// Tool Functions, TODO: VulkanUtils
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
    uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags props);
};
}