#include "VulkanRHI.h"
#include "Engine/Source/Runtime/Platform/FileSystem.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Snowy::Ark
{
void CreateDebugUtilsMessengerEXT(vk::Instance instance, const vk::DebugUtilsMessengerCreateInfoEXT& createInfo, vk::Optional<const vk::AllocationCallbacks> allocator,
                                        vk::DebugUtilsMessengerEXT* pCallback)
{
    VulkanUtils::ExecResult(instance.createDebugUtilsMessengerEXT(createInfo, allocator), "Failed to set up debug callback!", pCallback);
}
void DestoryDebugUtilsMessengerEXT(vk::Instance instance, vk::DebugUtilsMessengerEXT* pCallback, vk::Optional<const vk::AllocationCallbacks> allocator)
{
    instance.destroyDebugUtilsMessengerEXT(*pCallback, allocator);
}

VulkanRHI::VulkanRHI()
{
    InitWindow();
    InitVulkan();
}

void VulkanRHI::Run()
{
    MainLoop();
    Cleanup();
}

void VulkanRHI::InitWindow()
{
    LOG("Window Initialize, Start.");
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    m_Window = glfwCreateWindow(WIDTH, HEIGHT, "Hello Vulkan Triangle!", nullptr, nullptr);
    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, FramebufferResizeCallback);
    LOG("Window Initialize, Complete.");
}
void VulkanRHI::InitVulkan()
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
    CreateVertexBuffer();
    CreateSyncObjects();
    LOG("Vulkan Initialize, Complete.");
}
void VulkanRHI::MainLoop()
{
    while (!glfwWindowShouldClose(m_Window))
    {
        glfwPollEvents();
        DrawFrame();
    }
    VulkanUtils::ExecResult(m_Device.waitIdle(), "Failed to Wait Idle!");
}

void VulkanRHI::Cleanup()
{
    LOG("Vulkan Destory, Start.");

    CleanupSwapChain();

    m_Device.destroyBuffer(m_VertexBuffer);
    m_Device.freeMemory(m_VertexBufferMemory);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_Device.destroySemaphore(m_ImageAvailableSemaphores[i], nullptr);
        m_Device.destroySemaphore(m_RenderFinishedSemaphores[i], nullptr);
        m_Device.destroyFence(m_InFlightFences[i], nullptr);
    }
        
    m_Device.destroyCommandPool(m_CommandPool, nullptr);
    m_Device.destroy(nullptr);
    if (g_EnableValidationLayers)
    {
        DestoryDebugUtilsMessengerEXT(m_Instance, &m_Callback, nullptr);
    }

    m_Instance.destroySurfaceKHR(m_Surface, nullptr);
    m_Instance.destroy(nullptr);

    LOG("Vulkan Destory, Complete.");

    glfwDestroyWindow(m_Window);
    glfwTerminate();
}
void VulkanRHI::CreateInstance()
{
    LOG("Create Vulakn Instance, Start.");
    if (g_EnableValidationLayers && !CheckValidationLayerSupport())
    {
        throw std::runtime_error("Vaildation layers requested, but not available!");
    }

    vk::ApplicationInfo appInfo = {
        .pApplicationName = "Hello VkTriangle!",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "SnowyArk",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_1,
    };

    auto extensions = GetRequiredExtensions();

    vk::InstanceCreateInfo createInfo = {
        .pApplicationInfo = &appInfo,
    };
    createInfo.setPEnabledExtensionNames(extensions);
    if (g_EnableValidationLayers)
    {
        createInfo.setPEnabledLayerNames(g_ValidationLayers);
    } else
    {
        createInfo.setPEnabledLayerNames(nullptr);
    }

    VulkanUtils::ExecResult(vk::createInstance(createInfo, nullptr), "Failed to Create Vk Instance!", &m_Instance);

    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_Instance);
    LOG("Create Vulakn Instance, Complete.");
}
void VulkanRHI::SetupDebugCallback()
{
    LOG("Setup Debug Callback, Start.")
    if (!g_EnableValidationLayers)
    {
        return;
    }
    vk::DebugUtilsMessengerCreateInfoEXT createInfo = {
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        .pfnUserCallback = VulkanUtils::ValidationLayerDebugCallback,
        .pUserData = nullptr,
    };

    CreateDebugUtilsMessengerEXT(m_Instance, createInfo, nullptr, &m_Callback);
    LOG("Setup Debug Callback, Complete.")
}
void VulkanRHI::CreateSurface()
{
    LOG("Create Surface, Start.")
    if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, reinterpret_cast<decltype(m_Surface)::NativeType*>(&m_Surface)) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface!");
    }
    LOG("Create Surface, Complete.")
}
void VulkanRHI::PickPhysicalDevice()
{
    LOG("Pick Physical Device, Start.");
    VulkanUtils::ExecResult(m_Instance.enumeratePhysicalDevices(),
                            [this](const auto& result) {
                                auto& [r, v] = result;
                                if (r != vk::Result::eSuccess)
                                {
                                    throw std::runtime_error("Faild to find GPUs with Vulkan support!");
                                } else
                                {
                                    for (const auto& device : v)
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
                            });
    LOG("Pick Physical Device, Complete.");
}
void VulkanRHI::CreateLogicalDevice()
{
    LOG("Create Logical Device, Start.");
    QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(),
        indices.presentFamily.value()
    };

    float queuePriority = 1.0f;
    for (auto queueFamily : uniqueQueueFamilies)
    {
        vk::DeviceQueueCreateInfo queueCreateInfo = {
            .queueFamilyIndex = queueFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };
        queueCreateInfos.emplace_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures{};

    vk::DeviceCreateInfo createInfo = {
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(g_DeviceExtensions.size()),
        .ppEnabledExtensionNames = g_DeviceExtensions.data(),
        .pEnabledFeatures = &deviceFeatures,
    };
    if (g_EnableValidationLayers)
    {
        createInfo.setPEnabledLayerNames(g_ValidationLayers);
    } else
    {
        createInfo.setPEnabledLayerNames(nullptr);
    }

    VulkanUtils::ExecResult(m_PhysicalDevice.createDevice(createInfo, nullptr), "Failed to create logical device!", &m_Device);

    m_GraphicsQueue = m_Device.getQueue(indices.graphicsFamily.value(), 0);
    m_PresentQueue = m_Device.getQueue(indices.presentFamily.value(), 0);

    LOG("Create Logical Device, Complete.");
}

void VulkanRHI::CreateSwapChain()
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

    vk::SwapchainCreateInfoKHR createInfo = {
        .surface = m_Surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
    };

    QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);
    std::array<uint32_t, 2> queueFamilyIndices = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    if (indices.graphicsFamily.value() != indices.presentFamily.value())
    {
        createInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
            .setQueueFamilyIndices(queueFamilyIndices);
    } else
    {
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive)
            .setQueueFamilyIndices(nullptr);
    }

    createInfo.setPreTransform(swapChainSupport.capabilities.currentTransform)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setPresentMode(presentMode)
        .setClipped(RHI_TRUE)
        .setOldSwapchain(RHI_NULL_HANDLE);

    VulkanUtils::ExecResult(m_Device.createSwapchainKHR(createInfo, nullptr), "Failed to create swap chain!", &m_SwapChain);
    VulkanUtils::ExecResult(m_Device.getSwapchainImagesKHR(m_SwapChain), "Failed to get swap chain images!", &m_SwapChainImages);

    m_SwapChainImageFormat = surfaceFormat.format;
    m_SwapChainExtent = extent;

    LOG("Create SwapChain, Complete.");
}
void VulkanRHI::CreateImageViews()
{
    LOG("Create Image Views, Start.");
    m_SwapChainImageViews.resize(m_SwapChainImages.size());
    for (size_t i = 0; i < m_SwapChainImages.size(); i++)
    {
        vk::ImageViewCreateInfo createInfo = {
            .image = m_SwapChainImages[i],
            .viewType = vk::ImageViewType::e2D,
            .format = m_SwapChainImageFormat,
            .components = vk::ComponentMapping{},
            .subresourceRange = vk::ImageSubresourceRange {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        VulkanUtils::ExecResult(m_Device.createImageView(createInfo, nullptr),
                                std::format("Failed to create ImageView[{}]!", i), &m_SwapChainImageViews[i]);
    }
    LOG("Create Image Views, Complete.");
}
void VulkanRHI::CreateGraphicsPipeline()
{
    LOG("Create Graphics Pipeline, Start.");

    auto&& fs = FileSystem::Instance();
    auto vertShaderModule = CreateShaderModule(fs.ReadSpirvShaderBinary(fs.FullPath("Engine/Shaders/SPIR-V/vert.spv")));
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = vertShaderModule,
        .pName = "main",
    };
    auto fragShaderModule = CreateShaderModule(fs.ReadSpirvShaderBinary(fs.FullPath("Engine/Shaders/SPIR-V/frag.spv")));
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = fragShaderModule,
        .pName = "main",
    };

    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

    auto bindingDescription = Vertex::GetBindingDescription();
    auto attributeDescriptions = Vertex::GetAttributeDescriptions();
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<int32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data(),
    };

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = RHI_FALSE,
    };
        
    vk::Viewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(m_SwapChainExtent.width),
        .height = static_cast<float>(m_SwapChainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,  
    };

    vk::Rect2D scissor = {
        .offset = {0, 0},
        .extent = m_SwapChainExtent,
    };
        
    vk::PipelineViewportStateCreateInfo viewportState = {
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };
        
    vk::PipelineRasterizationStateCreateInfo rasterizer = {
        .depthClampEnable = RHI_FALSE,
        .rasterizerDiscardEnable = RHI_FALSE,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = RHI_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f,
    };
        
    vk::PipelineMultisampleStateCreateInfo multisampling = {
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = RHI_FALSE,
        .minSampleShading = 1,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = RHI_FALSE,
        .alphaToOneEnable = RHI_FALSE,
    };

    vk::PipelineColorBlendAttachmentState colorBlendAttachment =
    {
        .blendEnable = RHI_FALSE,
        .srcColorBlendFactor = vk::BlendFactor::eOne,
        .dstColorBlendFactor = vk::BlendFactor::eZero,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
    };
        
    vk::PipelineColorBlendStateCreateInfo colorBlending = {
        .logicOpEnable = RHI_FALSE,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = std::array{0.0f, 0.0f, 0.0f, 0.0f},
    };

    std::array<vk::DynamicState, 2> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eLineWidth };

    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data(),
    };
        
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };
        
    VulkanUtils::ExecResult(m_Device.createPipelineLayout(pipelineLayoutInfo, nullptr), "Failed to create pipeline layout!", &m_PipelineLayout);

    vk::GraphicsPipelineCreateInfo graphicsPipelineInfo = {
        .stageCount = static_cast<uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &colorBlending,
        .pDynamicState = nullptr,
        .layout = m_PipelineLayout,
        .renderPass = m_RenderPass,
        .subpass = 0,
        .basePipelineHandle = RHI_NULL_HANDLE,
        .basePipelineIndex = -1,
    };
        
    VulkanUtils::ExecResult(m_Device.createGraphicsPipeline(VK_NULL_HANDLE, graphicsPipelineInfo, nullptr),
                            "Failed to create graphics pipeline!", &m_GraphicsPipeline);

    m_Device.destroyShaderModule(vertShaderModule, nullptr);
    m_Device.destroyShaderModule(fragShaderModule, nullptr);
    LOG("Create Graphics Pipeline, Complete.")
}
void VulkanRHI::CreateRenderPass()
{
    LOG("Create Render Pass, Start.");

    vk::AttachmentDescription colorAttachmentDesc = {
        .flags = vk::AttachmentDescriptionFlags{},
        .format = m_SwapChainImageFormat,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::ePresentSrcKHR,
    };
        
    vk::AttachmentReference colorAttachmentRef = {
        .attachment = 0,
        .layout = vk::ImageLayout::eColorAttachmentOptimal,
    };

    vk::SubpassDescription subpassDesc = {
        .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
    };
        
    vk::SubpassDependency subpassDependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .srcAccessMask = vk::AccessFlagBits::eNone,
        .dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
    };
        
    vk::RenderPassCreateInfo renderPassInfo = {
        .attachmentCount = 1,
        .pAttachments = &colorAttachmentDesc,
        .subpassCount = 1,
        .pSubpasses = &subpassDesc,
        .dependencyCount = 1,
        .pDependencies = &subpassDependency,
    };
        
    VulkanUtils::ExecResult(m_Device.createRenderPass(renderPassInfo, nullptr), "Failed to create render pass!", &m_RenderPass);
    LOG("Create Render Pass, Complete.")
}
void VulkanRHI::CreateFramebuffers()
{
    LOG("Create Framebuffers, Start.")
    m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());
    for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
    {
        std::array<vk::ImageView, 1> attachments{ m_SwapChainImageViews[i] };

        vk::FramebufferCreateInfo framebufferInfo = {
            .renderPass = m_RenderPass,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = m_SwapChainExtent.width,
            .height = m_SwapChainExtent.height,
            .layers = 1,
        };
            
        VulkanUtils::ExecResult(m_Device.createFramebuffer(framebufferInfo, nullptr),
                                std::format("Failed to create framebuffer[{}]!", i), &m_SwapChainFramebuffers[i]);
    }
    LOG("Create Framebuffers, Complete.")
}
void VulkanRHI::CreateCommandPool()
{
    LOG("Create Command Pool, Start.")
    auto queueFamilyIndices = FindQueueFamilies(m_PhysicalDevice);

    vk::CommandPoolCreateInfo createInfo = {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(),
    };

    VulkanUtils::ExecResult(m_Device.createCommandPool(createInfo, nullptr), "Failed to create command pool!", &m_CommandPool);
    LOG("Create Command Pool, Complete.")
}
void VulkanRHI::CreateCommandBuffers()
{
    LOG("Create Command Buffers, Start.")
    m_CommandBuffers.resize(m_SwapChainFramebuffers.size());

    vk::CommandBufferAllocateInfo allocInfo = {
        .commandPool = m_CommandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size()),
    };
    VulkanUtils::ExecResult(m_Device.allocateCommandBuffers(allocInfo), "Failed to allocate command buffers!", &m_CommandBuffers);
    LOG("Create Command Buffers, Complete.")
}

void VulkanRHI::CreateVertexBuffer()
{
    vk::BufferCreateInfo bufferInfo = {
        .size = sizeof(g_TriangleVertices[0]) * g_TriangleVertices.size(),
        .usage = vk::BufferUsageFlagBits::eVertexBuffer,
        .sharingMode = vk::SharingMode::eExclusive,
    };
    VulkanUtils::ExecResult(m_Device.createBuffer(bufferInfo), "Failed to create vertex buffer!", &m_VertexBuffer);

    vk::MemoryRequirements memRequirements;
    m_Device.getBufferMemoryRequirements(m_VertexBuffer, &memRequirements);
    vk::MemoryAllocateInfo allocInfo = {
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent),
    };
    VulkanUtils::ExecResult(m_Device.allocateMemory(allocInfo), "Failed to allocate vertex buffer memory!", &m_VertexBufferMemory);
    VulkanUtils::ExecResult(m_Device.bindBufferMemory(m_VertexBuffer, m_VertexBufferMemory, 0), "Failed to bind vertex buffer memory!");

    void* data;
    VulkanUtils::ExecResult(m_Device.mapMemory(m_VertexBufferMemory, 0, bufferInfo.size, {}), "Failed to map vertex buffer memory!", &data);
    memcpy(data, g_TriangleVertices.data(), (size_t)bufferInfo.size);
    m_Device.unmapMemory(m_VertexBufferMemory);
}

void VulkanRHI::CreateSyncObjects()
{
    LOG("Create Semaphores, Start.")
    m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    vk::SemaphoreCreateInfo semaphoreInfo = {};
    vk::FenceCreateInfo fenceInfo = {
        .flags = vk::FenceCreateFlagBits::eSignaled,
    };

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VulkanUtils::ExecResult(m_Device.createSemaphore(semaphoreInfo, nullptr),
                                "Failed to create synchronization objects for a frame!", &m_ImageAvailableSemaphores[i]);

        VulkanUtils::ExecResult(m_Device.createSemaphore(semaphoreInfo, nullptr),
                                "Failed to create synchronization objects for a frame!", &m_RenderFinishedSemaphores[i]);

        VulkanUtils::ExecResult(m_Device.createFence(fenceInfo, nullptr),
                                "Failed to create synchronization objects for a frame!", &m_InFlightFences[i]);
    }
    LOG("Create Semaphores, Complete.")
}

void VulkanRHI::ReCreateSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_Window, &width, &height);
    while (width == 0 || height == 0) 
    {
        glfwGetFramebufferSize(m_Window, &width, &height);
        glfwWaitEvents();
    }

    auto _ = m_Device.waitIdle();

    CleanupSwapChain();

    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandBuffers();
}

void VulkanRHI::CleanupSwapChain()
{
    for (auto&& framebuffer : m_SwapChainFramebuffers)
    {
        m_Device.destroyFramebuffer(framebuffer, nullptr);
    }

    m_Device.freeCommandBuffers(m_CommandPool, m_CommandBuffers);

    m_Device.destroyPipeline(m_GraphicsPipeline, nullptr);
    m_Device.destroyPipelineLayout(m_PipelineLayout, nullptr);
    m_Device.destroyRenderPass(m_RenderPass, nullptr);

    for (auto&& imageView : m_SwapChainImageViews)
    {
        m_Device.destroyImageView(imageView, nullptr);
    }
    m_Device.destroySwapchainKHR(m_SwapChain, nullptr);
}

void VulkanRHI::RecordCommandBuffer(std::vector<vk::CommandBuffer>& cmds, uint32_t idx)
{
    vk::CommandBufferBeginInfo cmdBeginInfo = {
        .flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse,
        .pInheritanceInfo = nullptr,
    };

    VulkanUtils::ExecResult(cmds[idx].begin(cmdBeginInfo),
                            [&, this](auto result) {
                                if (result != vk::Result::eSuccess)
                                {
                                    throw std::runtime_error("Failed to begin recording command buffer!");
                                } else
                                {
                                    std::array<vk::ClearValue, 1> clearValues = {
                                        vk::ClearColorValue{.float32 = std::array{0.0f, 0.0f, 0.0f, 1.0f} }
                                    };
                                    vk::RenderPassBeginInfo renderPassBeginInfo = {
                                        .renderPass = m_RenderPass,
                                        .framebuffer = m_SwapChainFramebuffers[idx],
                                        .renderArea = vk::Rect2D {
                                            .offset = {0, 0},
                                            .extent = m_SwapChainExtent,
                                        },
                                        .clearValueCount = static_cast<uint32_t>(clearValues.size()),
                                        .pClearValues = clearValues.data(),
                                    };

                                    cmds[idx].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
                                    cmds[idx].bindPipeline(vk::PipelineBindPoint::eGraphics, m_GraphicsPipeline);

                                    std::array<vk::Buffer, 1> vertexBuffers = { m_VertexBuffer };
                                    std::array<vk::DeviceSize, 1> offsets = { 0 };
                                    cmds[idx].bindVertexBuffers(0, 1, vertexBuffers.data(), offsets.data());
                                    cmds[idx].draw(static_cast<uint32_t>(g_TriangleVertices.size()), 1, 0, 0);
                                    cmds[idx].endRenderPass();

                                    VulkanUtils::ExecResult(cmds[idx].end(), "Failed to end recording command buffer!");
                                };
                            });
}
void VulkanRHI::DrawFrame()
{
    auto waitForFencesResult = m_Device.waitForFences(m_InFlightFences[m_CurrentFrame], RHI_TRUE, std::numeric_limits<uint64_t>::max());

    uint32_t imageIndex;
    VulkanUtils::ExecResult(m_Device.acquireNextImageKHR(m_SwapChain, std::numeric_limits<uint64_t>::max(), m_ImageAvailableSemaphores[m_CurrentFrame], RHI_NULL_HANDLE, &imageIndex),
                            [this](auto result) {
                                if (result == vk::Result::eErrorOutOfDateKHR)
                                {
                                    ReCreateSwapChain();
                                    return;
                                } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
                                {
                                    throw std::runtime_error("Failed to acquire swap chain image!");
                                }
                            });

    m_Device.resetFences(m_InFlightFences[m_CurrentFrame]);

    RecordCommandBuffer(m_CommandBuffers, imageIndex);

    std::array<vk::PipelineStageFlags, 1> waitDstStageMask = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

    vk::SubmitInfo submitInfo = {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_ImageAvailableSemaphores[m_CurrentFrame],
        .pWaitDstStageMask = waitDstStageMask.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &m_CommandBuffers[imageIndex],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_RenderFinishedSemaphores[m_CurrentFrame],
    };

    VulkanUtils::ExecResult(m_GraphicsQueue.submit(submitInfo, m_InFlightFences[m_CurrentFrame]), "Failed to submit draw command buffer!");

    vk::PresentInfoKHR presentInfo = {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_RenderFinishedSemaphores[m_CurrentFrame],
        .swapchainCount = 1,
        .pSwapchains = &m_SwapChain,
        .pImageIndices = &imageIndex,
    };

    VulkanUtils::ExecResult(m_PresentQueue.presentKHR(presentInfo),
                            [this](auto result) {
                                if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || m_FramebufferResized)
                                {
                                    m_FramebufferResized = false;
                                    ReCreateSwapChain();
                                } else if (result != vk::Result::eSuccess)
                                {
                                    throw std::runtime_error("Failed to present swap chain image!");
                                }
                            });

    //vkQueueWaitIdle(m_PresentQueue);
    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

// ==============================================
// Tool Functions
// ==============================================
bool VulkanRHI::CheckValidationLayerSupport()
{
    LOG("Check Validation Layer Support, Start.");
    bool support = true;
    VulkanUtils::ExecResult(vk::enumerateInstanceLayerProperties(),
                            [&, this](const auto& result) {
                                auto& [r, v] = result;
                                if (r != vk::Result::eSuccess)
                                {
                                    throw std::runtime_error("Failed to enumerate instance layer properties!");
                                } else
                                {
                                    for (const char* layerName : g_ValidationLayers)
                                    {
                                        bool layerFound = false;
                                        for (const auto& layerProperies : v)
                                        {
                                            if (strcmp(layerName, layerProperies.layerName) == 0)
                                            {
                                                layerFound = true;
                                                break;
                                            }
                                        }
                                        if (!layerFound)
                                        {
                                            support = false;
                                        }
                                    }
                                }
                            });
    LOG("Check Validation Layer Support, Complete.");
    return support;
}
bool VulkanRHI::CheckDeviceExtensionSupport(vk::PhysicalDevice device)
{
    LOG("Check Device Extension Support, Start.");
    std::set<std::string> requiredExtensions(g_DeviceExtensions.begin(), g_DeviceExtensions.end());
    VulkanUtils::ExecResult(device.enumerateDeviceExtensionProperties(nullptr),
                            [&, this](const auto& result) {
                                auto& [r, v] = result;
                                if (r != vk::Result::eSuccess)
                                {
                                    throw std::runtime_error("Failed to enumerate instance layer properties!");
                                } else
                                {
                                    for (const auto& extension : v)
                                    {
                                        requiredExtensions.erase(extension.extensionName);
                                    }
                                }
                            });
    LOG("Check Device Extension Support, Complete.");
    return requiredExtensions.empty();
}
std::vector<const char*> VulkanRHI::GetRequiredExtensions()
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
bool VulkanRHI::IsDeviceSuitable(vk::PhysicalDevice device)
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
QueueFamilyIndices VulkanRHI::FindQueueFamilies(vk::PhysicalDevice device)
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
SwapChainSupportDetails VulkanRHI::QuerySwapChainSupport(vk::PhysicalDevice device)
{
    SwapChainSupportDetails details;
    // ��ѯ������������
    VulkanUtils::ExecResult(device.getSurfaceCapabilitiesKHR(m_Surface), "Failed to get Surface Capabilities!", &details.capabilities);
    // ��ѯ����֧�ָ�ʽ
    VulkanUtils::ExecResult(device.getSurfaceFormatsKHR(m_Surface), "Failed to get Surface Formats!", &details.formats);
    // ��ѯ֧�ֵĳ��ַ�ʽ
    VulkanUtils::ExecResult(device.getSurfacePresentModesKHR(m_Surface), "Failed to get Surface PresentModes!", &details.presentModes);
    return details;
}
vk::SurfaceFormatKHR VulkanRHI::ChooseSwapChainFormat(std::span<vk::SurfaceFormatKHR> availableFormats)
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
vk::PresentModeKHR VulkanRHI::ChooseSwapPresentMode(std::span<vk::PresentModeKHR> availablePresentModes)
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
vk::Extent2D VulkanRHI::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int actualWidth, actualHeight;
        glfwGetFramebufferSize(m_Window, &actualWidth, &actualHeight);
        vk::Extent2D actualExtent = {static_cast<uint32_t>(actualWidth), static_cast<uint32_t>(actualHeight)};
        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }
}
vk::ShaderModule VulkanRHI::CreateShaderModule(const std::vector<char>& code)
{
    vk::ShaderModule shaderModule;

    vk::ShaderModuleCreateInfo createInfo = {
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t*>(code.data()),
    };

    VulkanUtils::ExecResult(m_Device.createShaderModule(createInfo, nullptr), "Failed to create shader module!", &shaderModule);

    return shaderModule;
}
uint32_t VulkanRHI::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties props = m_PhysicalDevice.getMemoryProperties();
    for (uint32_t i = 0; i < props.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (props.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type!");
}
}