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
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texcoord;

    bool operator==(const SimpleVertex& other) const 
    {
        return position == other.position && color == other.color && texcoord == other.texcoord;
    }

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
            .format = vk::Format::eR32G32B32Sfloat,
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


static std::vector<SimpleVertex> g_TriangleVertices = {}/* = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
}*/;
static std::vector<uint32_t> g_TriangleIndices = {}/* = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
}*/;


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
    VulkanSwapchain m_Swapchain;
    ObserverHandle<GLFWwindow> m_WindowHandle;
    VulkanInstance m_Instance;
    VulkanDevice   m_Device;

    std::vector<vk::Framebuffer> m_SwapchainFramebuffers;
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

    std::vector<UniqueHandle<VulkanBuffer>> m_UniformBuffers;

    UniqueHandle<VulkanBuffer> m_VertexBuffer;
    UniqueHandle<VulkanBuffer> m_IndexBuffer;
    UniqueHandle<VulkanTexture> m_DepthAttachment;
    UniqueHandle<VulkanTexture> m_Texture;

public:
    VulkanInstance& GetInstance() noexcept { m_Instance; }
    VulkanDevice& GetDevice() noexcept { m_Device; }
    VulkanSwapchain& GetSwapchain() noexcept { m_Swapchain; }

    ObserverHandle<GLFWwindow> GetWindowHandle() const noexcept { return m_WindowHandle; }
    void SetWindowHandle(ObserverHandle<GLFWwindow> handle) noexcept { m_WindowHandle = handle; }

    vk::CommandBuffer BeginSingleTimeCommandBuffer();
    void EndSingleTimeCommandBuffer(vk::CommandBuffer cmd);

private:
    void Init_Internal(In<RHIConfig> config);
    void PostInit_Internal();

    void CreateInstance(Out<VulkanInstance> instance, In<RHIConfig> config) noexcept;

    void CleanupSwapChain();
    void RecreateSwapchain();

    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();  
    void CreateRenderPass();
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateCommandBuffers();

    void LoadModel(std::filesystem::path path);
    void CreateVertexBuffer(ArrayIn<SimpleVertex> triangleVertices);
    void CreateIndexBuffer(ArrayIn<uint32_t> triangleIndices);
    void CreateUniformBuffer();
    void CreateDepthAttachment();
    void CreateSampledTexture(std::filesystem::path path);

    void CreateDescriptorPool();
    void CreateDescriptorSets();
    void CreateSyncObjects();

    void RecordCommandBuffer(ArrayIn<vk::CommandBuffer> cmds, uint32_t idx);
    
    void DrawFrame();

// ==============================================
// Tool Functions
// ==============================================
public:
    void UpdateUniformBuffer(uint32_t idx);
    void CopyBuffer(In<VulkanBuffer> srcBuffer, Ref<VulkanBuffer> dstBuffer, vk::DeviceSize size);
    void CopyBufferToImage(In<VulkanBuffer> srcBuffer, Ref<VulkanTexture> dstImage, uint32_t width, uint32_t height);

    vk::Format FindSupportedFormat(ArrayIn<vk::Format> formats, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const noexcept;
    vk::Format GetDepthFormat() const noexcept;
};
}

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std {
    template<> struct hash<Snowy::Ark::SimpleVertex> {
        size_t operator()(Snowy::Ark::SimpleVertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texcoord) << 1);
        }
    };
}
