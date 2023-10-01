#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/RHI.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanUtils.h"

#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanAdapter.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanInstance.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanDevice.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanSwapchain.h"

#include <vulkan/vulkan.hpp>

//#include <GLFW/glfw3.h>
struct GLFWwindow;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <optional>

namespace Snowy::Ark
{
struct SACommonMatrices
{
    glm::mat4 SA_ObjectToWorld;
    glm::mat4 SA_MatrixV;
    glm::mat4 SA_MatrixP;
};

struct SimpleVertex
{
    glm::vec2 position;
    glm::vec3 color;
    glm::vec2 texcoord;

    static vk::VertexInputBindingDescription GetBindingDescription()
    {
        vk::VertexInputBindingDescription bindingDescription = {
            .binding = 0,
            .stride = sizeof(SimpleVertex),
            .inputRate = vk::VertexInputRate::eVertex,
        };
        return bindingDescription;
    }
    static std::array<vk::VertexInputAttributeDescription, 3> GetAttributeDescriptions()
    {
        std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions = {};
        attributeDescriptions[0] = {
            .location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32Sfloat,
            .offset = offsetof(SimpleVertex, position),
        };
        attributeDescriptions[1] = {
            .location = 1,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(SimpleVertex, color),
        };
        attributeDescriptions[2] = {
            .location = 2,
            .binding = 0,
            .format = vk::Format::eR32G32Sfloat,
            .offset = offsetof(SimpleVertex, texcoord),
        };
        return attributeDescriptions;
    }
};

static std::vector<SimpleVertex> g_TriangleVertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{ 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
};
static std::vector<uint16_t> g_TriangleIndices = { 0, 1, 2, 2, 3, 0 };


class VulkanRHI final : public RHI
{
public:
    VulkanRHI();
    ~VulkanRHI() override = default;
    VulkanRHI(const VulkanRHI&) = default;
    VulkanRHI(VulkanRHI&&) = default;
    VulkanRHI& operator=(const VulkanRHI&) = default;
    VulkanRHI& operator=(VulkanRHI&&) = default;

    void Init(In<RHIConfig> config) override;
    void Run() override;
    void Destory() override;

private:
    VulkanInstance  m_Instance;
    VulkanDevice    m_Device;
    VulkanSwapchain m_Swapchain;

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
    size_t m_CurrFrameIndex = 0;

    VulkanBuffer m_VertexBuffer, m_IndexBuffer;
    std::vector<VulkanBuffer> m_UniformBuffers;

    UniqueHandle<VulkanTexture> m_Texture;

public:
    vk::Device& Device() { return m_Device.Native(); }
    vk::Queue& GraphicsQueue() { return m_Device.Queue(ERHIQueue::Graphics); }
    vk::CommandPool& CommandPool() { return m_CommandPool; }

    ObserverHandle<GLFWwindow> GetWindowHandle() const { return m_Instance.GetWindowHandle(); }

    vk::CommandBuffer BeginSingleTimeCommandBuffer();
    void EndSingleTimeCommandBuffer(vk::CommandBuffer cmd);

private:
    void Init_Internal(In<RHIConfig> config);
    void PostInit_Internal();

    void CreateInstance(Out<VulkanInstance> instance, In<RHIConfig> config) noexcept;
    void CleanupSwapChain();
    void RecreateSwapchain();

    // ==============================================
    // Feature Functions
    // ==============================================
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();  
    void CreateRenderPass();
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateCommandBuffers();
    void CreateVertexBuffer(ArrayIn<SimpleVertex> triangleVertices);
    void CreateIndexBuffer(ArrayIn<uint16_t> triangleIndices);
    void CreateUniformBuffer();
    void CreateTexture(std::filesystem::path path);
    void CreateDescriptorPool();
    void CreateDescriptorSets();
    void CreateSyncObjects();

    void RecordCommandBuffer(ArrayIn<vk::CommandBuffer> cmds, uint32_t idx);
    
    void DrawFrame();

// ==============================================
// Tool Functions
// ==============================================
private:
    void UpdateUniformBuffer(uint32_t idx);
    void CopyBuffer(In<VulkanBuffer> srcBuffer, Ref<VulkanBuffer> dstBuffer, vk::DeviceSize size);
    void CopyBufferToImage(In<VulkanBuffer> srcBuffer, Ref<VulkanTexture> dstImage, uint32_t width, uint32_t height);
};
}
