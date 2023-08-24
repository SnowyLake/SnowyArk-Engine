#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/RHI.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanUtils.h"

// vulkan.hpp macros was pre define in /Engine/Source/Runtime/Core/Base/Define.h
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Snowy::Ark
{
struct UniformBufferObject
{
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix; 
    glm::mat4 projectMatrix;
};

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

static std::vector<Vertex> g_TriangleVertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{ 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{ 0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}}
};
static std::vector<uint16_t> g_TriangleIndices = { 0, 1, 2, 2, 3, 0 };

constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;
constexpr int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
constexpr bool g_EnableValidationLayers = false;
#else
constexpr bool g_EnableValidationLayers = true;
#endif

const std::vector<const char*> g_ValidationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> g_DeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

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
    using Utils = VulkanUtils;
public:
    VulkanRHI() = default;
    ~VulkanRHI() = default;
    void Init() override;
    void Run() override;
    void Destory() override;

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

    vk::DescriptorPool m_DescriptorPool;
    vk::DescriptorSetLayout m_DescriptorSetLayout;
    std::vector<vk::DescriptorSet> m_DescriptorSets;

    vk::PipelineLayout m_PipelineLayout;
    vk::Pipeline m_GraphicsPipeline;

    vk::CommandPool m_CommandPool;
    std::vector<vk::CommandBuffer> m_CommandBuffers;

    std::vector<vk::Semaphore> m_ImageAvailableSemaphores;
    std::vector<vk::Semaphore> m_RenderFinishedSemaphores;
    std::vector<vk::Fence> m_InFlightFences;
    size_t m_CurrentFrame = 0;
    bool m_FramebufferResized = false;

    vk::Buffer m_VertexBuffer, m_IndexBuffer;
    vk::DeviceMemory m_VertexBufferMemory, m_IndexBufferMemory;

    std::vector<vk::Buffer> m_UniformBuffers;
    std::vector<vk::DeviceMemory> m_UniformBuffersMemory;

public:
    vk::Device& Device() { return m_Device; }
    vk::Queue& GraphicsQueue() { return m_GraphicsQueue; }
    vk::CommandPool& CommandPool() { return m_CommandPool; }

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
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();  
    void CreateRenderPass();
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateCommandBuffers();
    void CreateVertexBuffer(ArrayIn<Vertex> triangleVertices);
    void CreateIndexBuffer(ArrayIn<uint16_t> triangleIndices);
    void CreateUniformBuffer();
    void CreateDescriptorPool();
    void CreateDescriptorSets();
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
// Tool Functions, TODO: Utils
// ==============================================
private:
    bool CheckValidationLayerSupport();
    bool CheckDeviceExtensionSupport(vk::PhysicalDevice device);
    std::vector<const char*> GetRequiredExtensions();
    bool IsDeviceSuitable(vk::PhysicalDevice device);
    QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice device);
    SwapChainSupportDetails QuerySwapChainSupport(vk::PhysicalDevice device);
    vk::SurfaceFormatKHR ChooseSwapChainFormat(ArrayIn<vk::SurfaceFormatKHR> availableFormats);
    vk::PresentModeKHR ChooseSwapPresentMode(ArrayIn<vk::PresentModeKHR> availablePresentModes);
    vk::Extent2D ChooseSwapExtent(In<vk::SurfaceCapabilitiesKHR> capabilities);
    vk::ShaderModule CreateShaderModule(ArrayIn<char> code);
    uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags props);
    void CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, Out<vk::Buffer> buffer, Out<vk::DeviceMemory> bufferMemory);
    void UpdateUniformBuffer(uint32_t idx);
};
}