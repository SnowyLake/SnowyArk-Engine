#include "Engine/Source/Runtime/Core/Base/Common.h"

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#pragma comment(lib, "vulkan-1.lib")
#pragma comment(lib, "glfw3.lib")

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

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

#ifdef NDEBUG
constexpr bool g_EnableValidationLayers = false;
#else
constexpr bool g_EnableValidationLayers = true;
#endif



void CreateDebugUtilsMessengerEXT(vk::Instance instance, const vk::DebugUtilsMessengerCreateInfoEXT& createInfo, vk::Optional<const vk::AllocationCallbacks> allocator,
                                        vk::DebugUtilsMessengerEXT* pCallback)
{
    auto&& [result, callback] = instance.createDebugUtilsMessengerEXT(createInfo, allocator);
    CHECK_VK_RESULT(result, "Failed to set up debug callback!")
    {
        *pCallback = callback;
    }
}
void DestoryDebugUtilsMessengerEXT(vk::Instance instance, vk::DebugUtilsMessengerEXT* pCallback, vk::Optional<const vk::AllocationCallbacks> allocator)
{
    instance.destroyDebugUtilsMessengerEXT(*pCallback, allocator);
}
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

class Application
{
public:
    Application()
    {
        InitWindow();
        InitVulkan();
    }

    void Run()
    {
        MainLoop();
        Cleanup();
    }
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
    void InitWindow()
    {
        LOG("Window Initialize, Start.");
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        m_Window = glfwCreateWindow(WIDTH, HEIGHT, "Hello VkTriangle!", nullptr, nullptr);
        LOG("Window Initialize, Complete.");
    }
    void InitVulkan()
    {
        LOG("Vulkan Initialize, Start.");

        //TODO: PreInitVulkan
        vk::DynamicLoader dl;
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>( "vkGetInstanceProcAddr");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

        CreateInstance();
        SetupDebugCallback();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDevice();
        CreateSwapChain();
        CreateImageViews();
        CreateRenderPass();
        CreateGraphicsPipeline();
        CreateFramebuffers();
        CreateCommandPool();
        CreateCommandBuffers();
        CreateSyncObjects();
        LOG("Vulkan Initialize, Complete.");
    }
    void MainLoop()
    {
        while (!glfwWindowShouldClose(m_Window))
        {
            glfwPollEvents();
            DrawFrame();
        }
        auto&& result = m_Device.waitIdle();
        CHECK_VK_RESULT(result, "Failed to Wait Idle!");
    }
    void Cleanup()
    {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_Device.destroySemaphore(m_ImageAvailableSemaphores[i], nullptr);
            m_Device.destroySemaphore(m_RenderFinishedSemaphores[i], nullptr);
            m_Device.destroyFence(m_InFlightFences[i], nullptr);
        }
        
        m_Device.destroyCommandPool(m_CommandPool, nullptr);

        for (auto&& framebuffer : m_SwapChainFramebuffers)
        {
            m_Device.destroyFramebuffer(framebuffer, nullptr);
        }

        m_Device.destroyPipeline(m_GraphicsPipeline, nullptr);
        m_Device.destroyPipelineLayout(m_PipelineLayout, nullptr);
        m_Device.destroyRenderPass(m_RenderPass, nullptr);

        for (auto imageView : m_SwapChainImageViews)
        {
            m_Device.destroyImageView(imageView, nullptr);
        }
        m_Device.destroySwapchainKHR(m_SwapChain, nullptr);
        m_Device.destroy(nullptr);
        if (g_EnableValidationLayers)
        {
            DestoryDebugUtilsMessengerEXT(m_Instance, &m_Callback, nullptr);
        }

        m_Instance.destroySurfaceKHR(m_Surface, nullptr);
        m_Instance.destroy(nullptr);

        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    // ==============================================
    // Feature Functions
    // ==============================================
    void CreateInstance()
    {
        LOG("Create Vulakn Instance, Start.");
        if (g_EnableValidationLayers && !CheckValidationLayerSupport())
        {
            throw std::runtime_error("Vaildation layers requested, but not available!");
        }

        auto&& appInfo = vk::ApplicationInfo()
            .setPApplicationName("Hello VkTriangle!")
            .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
            .setPEngineName("SnowyArk")
            .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
            .setApiVersion(VK_API_VERSION_1_1);

        auto&& extensions = GetRequiredExtensions();

        auto&& createInfo = vk::InstanceCreateInfo()
            .setPApplicationInfo(&appInfo)
            .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
            .setPpEnabledExtensionNames(extensions.data());

        if (g_EnableValidationLayers)
        {
            createInfo.setEnabledLayerCount(static_cast<uint32_t>(g_ValidationLayers.size()))
                .setPpEnabledLayerNames(g_ValidationLayers.data());
        } else
        {
            createInfo.setEnabledLayerCount(0);
        }
        auto&& [createInstanceResult, instance] = vk::createInstance(createInfo, nullptr);
        CHECK_VK_RESULT(createInstanceResult, "Failed to Create Vk Instance!")
        {
            m_Instance = instance;
        }
        
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_Instance);
        LOG("Create Vulakn Instance, Complete.");
    }

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, 
                                                          const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        std::cerr << std::format("Validation layer: {}\n", pCallbackData->pMessage);
        return VK_FALSE;
    }

    void SetupDebugCallback()
    {
        LOG("Setup Debug Callback, Start.")
        if (!g_EnableValidationLayers)
        {
            return;
        }
        auto&& createInfo = vk::DebugUtilsMessengerCreateInfoEXT()
            .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
            .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
            .setPfnUserCallback(DebugCallback)
            .setPUserData(nullptr);

        CreateDebugUtilsMessengerEXT(m_Instance, createInfo, nullptr, &m_Callback);
        LOG("Setup Debug Callback, Complete.")
    }

    void CreateSurface()
    {
        LOG("Create Surface, Start.")
        if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, reinterpret_cast<VkSurfaceKHR*>(&m_Surface)) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create window surface!");
        }
        LOG("Create Surface, Complete.")
    }

    void PickPhysicalDevice()
    {
        LOG("Pick Physical Device, Start.");
        auto&& [result, devices] = m_Instance.enumeratePhysicalDevices();
        CHECK_VK_RESULT(result, "Faild to find GPUs with Vulkan support!");
        {
            for (const auto& device : devices)
            {
                if (IsDeviceSuitable(device))
                {
                    m_PhysicalDevice = device;
                    break;
                }
            }
            if (!m_PhysicalDevice)
            {
                throw std::runtime_error("Failed to find a suitable GPU!");
            }
        }
        LOG("Pick Physical Device, Complete.");
    }

    void CreateLogicalDevice()
    {
        LOG("Create Logical Device, Start.");
        QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
            indices.graphicsFamily.value(),
            indices.presentFamily.value()
        };

        float queuePriority = 1.0f;
        for (int queueFamily : uniqueQueueFamilies)
        {
            auto&& queueCreateInfo = vk::DeviceQueueCreateInfo()
                .setQueueFamilyIndex(queueFamily)
                .setQueueCount(1)
                .setPQueuePriorities(&queuePriority);
            queueCreateInfos.emplace_back(queueCreateInfo);
        }

        vk::PhysicalDeviceFeatures deviceFeatures{};

        auto&& createInfo = vk::DeviceCreateInfo()
            .setPQueueCreateInfos(queueCreateInfos.data())
            .setQueueCreateInfoCount(1)
            .setPEnabledFeatures(&deviceFeatures)
            .setEnabledExtensionCount(static_cast<uint32_t>(g_DeviceExtensions.size()))
            .setPpEnabledExtensionNames(g_DeviceExtensions.data());
        if (g_EnableValidationLayers)
        {
            createInfo.setEnabledLayerCount(static_cast<uint32_t>(g_ValidationLayers.size()))
                .setPpEnabledLayerNames(g_ValidationLayers.data());
        } else
        {
            createInfo.setEnabledLayerCount(0);
        }

        auto&& [result, device] = m_PhysicalDevice.createDevice(createInfo, nullptr);
        CHECK_VK_RESULT(result, "Failed to create logical device!")
        {
            m_Device = device;
        }

        m_GraphicsQueue = m_Device.getQueue(indices.graphicsFamily.value(), 0);
        m_PresentQueue = m_Device.getQueue(indices.presentFamily.value(), 0);

        LOG("Create Logical Device, Complete.");
    }

    void CreateSwapChain()
    {
        LOG("Create SwapChain, Start.");
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice);
        vk::SurfaceFormatKHR surfaceFormat = ChooseSwapChainFormat(swapChainSupport.formats);
        vk::PresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
        vk::Extent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        auto&& createInfo = vk::SwapchainCreateInfoKHR()
            .setSurface(m_Surface)
            .setMinImageCount(imageCount)
            .setImageFormat(surfaceFormat.format)
            .setImageColorSpace(surfaceFormat.colorSpace)
            .setImageExtent(extent)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

        QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        if (indices.graphicsFamily.value() != indices.presentFamily.value())
        {
            createInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
                .setQueueFamilyIndexCount(2)
                .setPQueueFamilyIndices(queueFamilyIndices);
        } else
        {
            createInfo.setImageSharingMode(vk::SharingMode::eExclusive)
                .setQueueFamilyIndexCount(0)
                .setPQueueFamilyIndices(nullptr);
        }

        createInfo.setPreTransform(swapChainSupport.capabilities.currentTransform)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(presentMode)
            .setClipped(VK_TRUE)
            .setOldSwapchain(VK_NULL_HANDLE);

        auto&& [createSwapchainResult, swapChain] = m_Device.createSwapchainKHR(createInfo, nullptr);
        CHECK_VK_RESULT(createSwapchainResult, "Failed to create swap chain!")
        {
            m_SwapChain = swapChain;
        }

        auto&& [getSwapchainImagesResult, images] = m_Device.getSwapchainImagesKHR(m_SwapChain);
        CHECK_VK_RESULT(getSwapchainImagesResult, "Failed to get swap chain images!")
        {
            m_SwapChainImages = images;
        }

        m_SwapChainImageFormat = surfaceFormat.format;
        m_SwapChainExtent = extent;

        LOG("Create SwapChain, Complete.");
    }

    void CreateImageViews()
    {
        LOG("Create Image Views, Start.");
        m_SwapChainImageViews.resize(m_SwapChainImages.size());
        for (size_t i = 0; i < m_SwapChainImages.size(); i++)
        {
            auto&& createInfo = vk::ImageViewCreateInfo()
                .setImage(m_SwapChainImages[i])
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(m_SwapChainImageFormat)
                .setComponents(vk::ComponentMapping{})
                .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

            auto&& [result, imageView] = m_Device.createImageView(createInfo, nullptr);
            CHECK_VK_RESULT(result, std::format("Failed to create ImageView[{}]!", i))
            {
                m_SwapChainImageViews[i] = imageView;
            }
        }
        LOG("Create Image Views, Complete.")
    }

    void CreateGraphicsPipeline()
    {
        LOG("Create Graphics Pipeline, Start.")

        auto vertShaderModule = CreateShaderModule(ReadFile("Engine/Shaders/Generated/vert.spv"));
        auto&& vertShaderStageInfo = vk::PipelineShaderStageCreateInfo()
            .setStage(vk::ShaderStageFlagBits::eVertex)
            .setModule(vertShaderModule)
            .setPName("main");

        auto fragShaderModule = CreateShaderModule(ReadFile("Engine/Shaders/Generated/frag.spv"));
        auto&& fragShaderStageInfo = vk::PipelineShaderStageCreateInfo()
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setModule(fragShaderModule)
            .setPName("main");
        
        std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

        auto&& vertexInputInfo = vk::PipelineVertexInputStateCreateInfo()
            .setVertexBindingDescriptionCount(0)
            .setPVertexBindingDescriptions(nullptr)
            .setVertexAttributeDescriptionCount(0)
            .setPVertexAttributeDescriptions(nullptr);

        auto&& inputAssembly = vk::PipelineInputAssemblyStateCreateInfo()
            .setTopology(vk::PrimitiveTopology::eTriangleList)
            .setPrimitiveRestartEnable(VK_FALSE);
        
        auto&& viewport = vk::Viewport()
            .setX(0.0f)
            .setY(0.0f)
            .setWidth(static_cast<float>(m_SwapChainExtent.width))
            .setHeight(static_cast<float>(m_SwapChainExtent.height))
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f);

        auto&& scissor = vk::Rect2D()
            .setOffset({0, 0})
            .setExtent(m_SwapChainExtent);

        auto&& viewportState = vk::PipelineViewportStateCreateInfo()
            .setViewportCount(1)
            .setPViewports(&viewport)
            .setScissorCount(1)
            .setScissors(scissor);

        auto&& rasterizer = vk::PipelineRasterizationStateCreateInfo()
            .setDepthClampEnable(VK_FALSE)
            .setRasterizerDiscardEnable(VK_FALSE)
            .setPolygonMode(vk::PolygonMode::eFill)
            .setCullMode(vk::CullModeFlagBits::eBack)
            .setFrontFace(vk::FrontFace::eClockwise)
            .setDepthBiasEnable(VK_FALSE)
            .setDepthBiasConstantFactor(0.0f)
            .setDepthBiasClamp(0.0f)
            .setDepthBiasSlopeFactor(0.0f)
            .setLineWidth(1.0f);

        auto&& multisampling = vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(vk::SampleCountFlagBits::e1)
            .setSampleShadingEnable(VK_FALSE)
            .setMinSampleShading(1)
            .setPSampleMask(nullptr)
            .setAlphaToCoverageEnable(VK_FALSE)
            .setAlphaToOneEnable(VK_FALSE);

        auto&& colorBlendAttachment = vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(VK_FALSE)
            .setSrcColorBlendFactor(vk::BlendFactor::eOne)
            .setDstColorBlendFactor(vk::BlendFactor::eZero)
            .setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
            .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
            .setAlphaBlendOp(vk::BlendOp::eAdd)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

        auto&& colorBlending = vk::PipelineColorBlendStateCreateInfo()
            .setLogicOpEnable(VK_FALSE)
            .setLogicOp(vk::LogicOp::eCopy)
            .setAttachmentCount(1)
            .setPAttachments(&colorBlendAttachment)
            .setBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});

        std::array<vk::DynamicState, 2> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eLineWidth };
        auto&& dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo()
            .setDynamicStateCount(2)
            .setPDynamicStates(dynamicStates.data());

        auto&& pipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
            .setSetLayoutCount(0)
            .setPSetLayouts(nullptr)
            .setPushConstantRangeCount(0)
            .setPPushConstantRanges(nullptr);

        auto&& [createPipelineLayoutResult, pipelineLayout] = m_Device.createPipelineLayout(pipelineLayoutInfo, nullptr);
        CHECK_VK_RESULT(createPipelineLayoutResult, "Failed to create pipeline layout!")
        {
            m_PipelineLayout = pipelineLayout;
        }

        auto&& graphicsPipelineInfo = vk::GraphicsPipelineCreateInfo()
            .setStageCount(2)
            .setPStages(shaderStages.data())
            .setPVertexInputState(&vertexInputInfo)
            .setPInputAssemblyState(&inputAssembly)
            .setPViewportState(&viewportState)
            .setPRasterizationState(&rasterizer)
            .setPMultisampleState(&multisampling)
            .setPDepthStencilState(nullptr)
            .setPColorBlendState(&colorBlending)
            .setPDynamicState(nullptr)
            .setLayout(m_PipelineLayout)
            .setRenderPass(m_RenderPass)
            .setSubpass(0)
            .setBasePipelineHandle(VK_NULL_HANDLE)
            .setBasePipelineIndex(-1);

        auto&& [createGraphicsPipelineResult, graphicsPipeline] = m_Device.createGraphicsPipeline(VK_NULL_HANDLE, graphicsPipelineInfo, nullptr);
        CHECK_VK_RESULT(createGraphicsPipelineResult, "Failed to create graphics pipeline!")
        {
            m_GraphicsPipeline = graphicsPipeline;
        }

        m_Device.destroyShaderModule(vertShaderModule, nullptr);
        m_Device.destroyShaderModule(fragShaderModule, nullptr);
        LOG("Create Graphics Pipeline, Complete.")
    }
    
    void CreateRenderPass()
    {
        LOG("Create Render Pass, Start.")

        auto&& colorAttachmentDesc = vk::AttachmentDescription()
            .setFormat(m_SwapChainImageFormat)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

        auto&& colorAttachmentRef = vk::AttachmentReference()
            .setAttachment(0)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

        auto&& subpassDesc = vk::SubpassDescription()
            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachmentCount(1)
            .setPColorAttachments(&colorAttachmentRef);

        auto&& subpassDependency = vk::SubpassDependency()
            .setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setSrcAccessMask(vk::AccessFlagBits::eNone)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

        auto&& renderPassInfo = vk::RenderPassCreateInfo()
            .setAttachmentCount(1)
            .setPAttachments(&colorAttachmentDesc)
            .setSubpassCount(1)
            .setPSubpasses(&subpassDesc)
            .setDependencyCount(1)
            .setPDependencies(&subpassDependency);

        auto&& [createRenderPassResult, renderPass] = m_Device.createRenderPass(renderPassInfo, nullptr);
        CHECK_VK_RESULT(createRenderPassResult, "Failed to create render pass!");
        {
            m_RenderPass = renderPass;
        }
        LOG("Create Render Pass, Complete.")
    }

    void CreateFramebuffers()
    {
        LOG("Create Framebuffers, Start.")
        m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());
        for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
        {
            std::array<vk::ImageView, 1> attachments{ m_SwapChainImageViews[i] };

            auto&& framebufferInfo = vk::FramebufferCreateInfo()
                .setRenderPass(m_RenderPass)
                .setAttachmentCount(1)
                .setPAttachments(attachments.data())
                .setWidth(m_SwapChainExtent.width)
                .setHeight(m_SwapChainExtent.height)
                .setLayers(1);

            auto&& [createFramebufferResult, swapChainFramebuffer] = m_Device.createFramebuffer(framebufferInfo, nullptr);
            CHECK_VK_RESULT(createFramebufferResult, std::format("Failed to create framebuffer[{}]!", i))
            {
                m_SwapChainFramebuffers[i] = swapChainFramebuffer;
            }
        }
        LOG("Create Framebuffers, Complete.")
    }

    void CreateCommandPool()
    {
        LOG("Create Command Pool, Start.")
        auto queueFamilyIndices = FindQueueFamilies(m_PhysicalDevice);

        auto&& createInfo = vk::CommandPoolCreateInfo()
            .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
            .setQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value());

        auto&& [result, cmdPool] = m_Device.createCommandPool(createInfo, nullptr);
        CHECK_VK_RESULT(result, "Failed to create command pool!")
        {
            m_CommandPool = cmdPool;
        }
        LOG("Create Command Pool, Complete.")
    }

    void CreateCommandBuffers()
    {
        LOG("Create Command Buffers, Start.")
        m_CommandBuffers.resize(m_SwapChainFramebuffers.size());

        auto&& allocInfo = vk::CommandBufferAllocateInfo()
            .setCommandPool(m_CommandPool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(static_cast<uint32_t>(m_CommandBuffers.size()));

        auto&& [allocResult, cmds] = m_Device.allocateCommandBuffers(allocInfo);
        CHECK_VK_RESULT(allocResult, "Failed to allocate command buffers!")
        {
            m_CommandBuffers = std::move(cmds);
        }
        LOG("Create Command Buffers, Complete.")
    }

    void CreateSyncObjects()
    {
        LOG("Create Semaphores, Start.")
        m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        auto&& semaphoreInfo = vk::SemaphoreCreateInfo();
        auto&& fenceInfo = vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            auto&& [createImageAvailableSemaphoreResult, imageAvailableSemaphore] = m_Device.createSemaphore(semaphoreInfo, nullptr);
            CHECK_VK_RESULT(createImageAvailableSemaphoreResult, "Failed to create synchronization objects for a frame!")
            {
                m_ImageAvailableSemaphores[i] = imageAvailableSemaphore;
            }
            auto&& [createRenderFinishedSemaphoreResult, renderFinishedSemaphore] = m_Device.createSemaphore(semaphoreInfo, nullptr);
            CHECK_VK_RESULT(createRenderFinishedSemaphoreResult, "Failed to create synchronization objects for a frame!")
            {
                m_RenderFinishedSemaphores[i] = renderFinishedSemaphore;
            }
            auto&& [createInFlightFencesResult, inFlightFences] = m_Device.createFence(fenceInfo, nullptr);
            CHECK_VK_RESULT(createInFlightFencesResult, "Failed to create synchronization objects for a frame!")
            {
                m_InFlightFences[i] = inFlightFences;
            }

        }
        LOG("Create Semaphores, Complete.")
    }

    void RecordCommandBuffer(std::vector<vk::CommandBuffer>& commandBuffers, uint32_t imageIndex)
    {
        auto&& cmdBeginInfo = vk::CommandBufferBeginInfo()
            .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse)
            .setPInheritanceInfo(nullptr);

        auto beginResult = commandBuffers[imageIndex].begin(cmdBeginInfo);
        CHECK_VK_RESULT(beginResult, "Failed to begin recording command buffer!");

        std::array<vk::ClearValue, 1> clearValues = { vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f} };
        auto&& renderPassBeginInfo = vk::RenderPassBeginInfo()
            .setRenderPass(m_RenderPass)
            .setFramebuffer(m_SwapChainFramebuffers[imageIndex])
            .setRenderArea(vk::Rect2D().setOffset({0,0}).setExtent(m_SwapChainExtent))
            .setClearValues(clearValues);

        commandBuffers[imageIndex].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
        commandBuffers[imageIndex].bindPipeline(vk::PipelineBindPoint::eGraphics, m_GraphicsPipeline);
        commandBuffers[imageIndex].draw(3, 1, 0, 0);
        commandBuffers[imageIndex].endRenderPass();

        auto endResult = commandBuffers[imageIndex].end();
        CHECK_VK_RESULT(endResult, "Failed to begin recording command buffer!");
    }
    
    void DrawFrame()
    {
        auto&& waitForFencesResult = m_Device.waitForFences(m_InFlightFences[m_CurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
        m_Device.resetFences(m_InFlightFences[m_CurrentFrame]);

        uint32_t imageIndex;
        auto&& acquireNextImageResult = m_Device.acquireNextImageKHR(m_SwapChain, std::numeric_limits<uint64_t>::max(), m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

        RecordCommandBuffer(m_CommandBuffers, imageIndex);

        std::array<vk::PipelineStageFlags, 1> waitDstStageMask = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

        auto&& submitInfo = vk::SubmitInfo()
            .setWaitSemaphores(m_ImageAvailableSemaphores[m_CurrentFrame])
            .setSignalSemaphores(m_RenderFinishedSemaphores[m_CurrentFrame])
            .setCommandBuffers(m_CommandBuffers[imageIndex])
            .setWaitDstStageMask(waitDstStageMask);

        auto&& submitResult = m_GraphicsQueue.submit(submitInfo, m_InFlightFences[m_CurrentFrame]);
        CHECK_VK_RESULT(submitResult, "Failed to submit draw command buffer!");

        auto&& presentInfo = vk::PresentInfoKHR()
            .setWaitSemaphores(m_RenderFinishedSemaphores[m_CurrentFrame])
            .setSwapchains(m_SwapChain)
            .setImageIndices(imageIndex);

        auto&& presentResult = m_PresentQueue.presentKHR(presentInfo);
        CHECK_VK_RESULT(presentResult, "Failed to present image!");

        //vkQueueWaitIdle(m_PresentQueue);
        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

// ==============================================
// Tool Functions
// ==============================================
private:
    bool CheckValidationLayerSupport()
    {
        LOG("Check Validation Layer Support, Start.");

        auto&& [result, availableLayers] = vk::enumerateInstanceLayerProperties();
        CHECK_VK_RESULT(result, "Failed to enumerate instance layer properties!");

        for (const char* layerName : g_ValidationLayers)
        {
            bool layerFound = false;
            for (const auto& layerProperies : availableLayers)
            {
                if (strcmp(layerName, layerProperies.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound)
            {
                return false;
            }
        }

        LOG("Check Validation Layer Support, Complete.");
        return true;
    }

    bool CheckDeviceExtensionSupport(vk::PhysicalDevice device)
    {
        LOG("Check Device Extension Support, Start.");

        auto&& [result, availableExtensions] = device.enumerateDeviceExtensionProperties(nullptr);
        CHECK_VK_RESULT(result, "Failed to enumerate device extension properties!");

        std::set<std::string> requiredExtensions(g_DeviceExtensions.begin(), g_DeviceExtensions.end());
        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        LOG("Check Device Extension Support, Complete.");
        return requiredExtensions.empty();
    }

    std::vector<const char*> GetRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (g_EnableValidationLayers)
        {
            extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        return extensions;
    }

    bool IsDeviceSuitable(vk::PhysicalDevice device)
    {
        QueueFamilyIndices indices = FindQueueFamilies(device);
        bool extensionsSupported = CheckDeviceExtensionSupport(device);
        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }
        return indices.IsComplete() && extensionsSupported && swapChainAdequate;
    }

    QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice device)
    {
        QueueFamilyIndices indices;

        std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

        int idx = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                indices.graphicsFamily = idx;
            }

            vk::Bool32 presentSupport = device.getSurfaceSupportKHR(idx, m_Surface).value;
            if (queueFamily.queueCount > 0 && presentSupport)
            {
                indices.presentFamily = idx;
            }

            if (indices.IsComplete())
            {
                break;
            }
            idx++;
        }
        return indices;
    }

    SwapChainSupportDetails QuerySwapChainSupport(vk::PhysicalDevice device)
    {
        SwapChainSupportDetails details;

        // 查询基础表面特性
        auto&& [getCapabilitiesResult, capabilities] = device.getSurfaceCapabilitiesKHR(m_Surface);
        CHECK_VK_RESULT(getCapabilitiesResult, "Failed to get Surface Capabilities!")
        {
            details.capabilities = capabilities;
        }
        // 查询表面支持格式
        auto&& [getFormatsResult, formats] = device.getSurfaceFormatsKHR(m_Surface);
        CHECK_VK_RESULT(getFormatsResult, "Failed to get Surface Formats!")
        {
            details.formats = formats;
        }
        // 查询支持的呈现方式
        auto&& [getPresentModesResult, presentModes] = device.getSurfacePresentModesKHR(m_Surface);
        CHECK_VK_RESULT(getPresentModesResult, "Failed to get Surface PresentModes!")
        {
            details.presentModes = presentModes;
        }

        return details;
    }

    vk::SurfaceFormatKHR ChooseSwapChainFormat(std::span<vk::SurfaceFormatKHR> availableFormats)
    {
        if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined)
        {
            return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
        }

        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }
    vk::PresentModeKHR ChooseSwapPresentMode(std::span<vk::PresentModeKHR> availablePresentModes)
    {
        auto bestMode = vk::PresentModeKHR::eFifo;
        for (const auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox)
            {
                return availablePresentMode;
            }
            else
            {
                bestMode = vk::PresentModeKHR::eImmediate;
            }
        }
        return bestMode;
    }
    vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            vk::Extent2D actualExtent = {WIDTH, HEIGHT};
            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
            return actualExtent;
        }
    }

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

    VkShaderModule CreateShaderModule(const std::vector<char>& code)
    {
        auto&& createInfo = vk::ShaderModuleCreateInfo()
            .setCodeSize(code.size())
            .setPCode(reinterpret_cast<const uint32_t*>(code.data()));

        auto&& [result, shaderModule] = m_Device.createShaderModule(createInfo, nullptr);
        CHECK_VK_RESULT(result, "Failed to create shader module!");

        return shaderModule;
    }
};
}

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