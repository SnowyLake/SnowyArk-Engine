#include "VulkanRHI.h"
#include "Engine/Source/Runtime/Core/Log/Logger.h"
#include "Engine/Source/Runtime/Function/Global/GlobalContext.h"
#include "Engine/Source/Runtime/Function/Window/WindowSystem.h"
#include "Engine/Source/Runtime/Platform/FileSystem.h"

#include <set>

namespace Snowy::Ark
{
using Utils = VulkanUtils;

void VulkanRHI::Init(Ref<RHIConfig> config)
{
    PreInit_Internal(config);
    Init_Internal();
    PostInit_Internal();
    SA_LOG_INFO(STEXT("Vulkan Context Initialized."));
}

void VulkanRHI::Run()
{
    DrawFrame();
}

void VulkanRHI::PreInit_Internal(Ref<RHIConfig> config)
{
    m_WindowHandle = config.windowHandle;
    m_MaxFrameInFlight = config.maxFrameInFlight;

    m_Instance.PrepareExtensionsAndLayers(config);
    m_Device.PrepareExtensionsAndLayers(config);
}

void VulkanRHI::Init_Internal()
{
    CreateInstance();
    CreateLogicalDevice();

    CreateSwapChain();
}

void VulkanRHI::PostInit_Internal()
{
    CreateImageViews();
    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateFramebuffers();

    CreateCommandPool();

    CreateVertexBuffer(g_TriangleVertices);
    CreateIndexBuffer(g_TriangleIndices);
    CreateUniformBuffer();

    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffers();

    CreateSyncObjects();
}

void VulkanRHI::Destory()
{
    Utils::VerifyResult(m_Device->waitIdle(), STEXT("Failed to Wait Idle!"));

    CleanupSwapChain();

    m_Device->destroyDescriptorSetLayout(m_DescriptorSetLayout);
    m_Device->destroyDescriptorPool(m_DescriptorPool);

    m_Device->destroyBuffer(m_VertexBuffer);
    m_Device->freeMemory(m_VertexBufferMemory);
    m_Device->destroyBuffer(m_IndexBuffer);
    m_Device->freeMemory(m_IndexBufferMemory);
    for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
    {
        m_Device->destroyBuffer(m_UniformBuffers[i]);
        m_Device->freeMemory(m_UniformBuffersMemory[i]);
    }

    for (size_t i = 0; i < m_MaxFrameInFlight; i++)
    {
        m_Device->destroySemaphore(m_ImageAvailableSemaphores[i]);
        m_Device->destroySemaphore(m_RenderFinishedSemaphores[i]);
        m_Device->destroyFence(m_InFlightFences[i]);
    }

    m_Device->destroyCommandPool(m_CommandPool);

    m_Device.Destroy();
    m_Instance.Destroy();

    SA_LOG_INFO(STEXT("Vulkan Context Destoryed."));
}

void VulkanRHI::CreateInstance()
{
    vk::ApplicationInfo appInfo = {
        .pApplicationName = "VulkanRHI",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "SnowyArk",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_1,
    };

    vk::InstanceCreateInfo createInfo = {
        .pApplicationInfo = &appInfo,
    };

    m_Instance.Init(this, createInfo);
    SA_LOG_INFO(STEXT("Vulkan Instance Initialized."));
}

void VulkanRHI::CreateLogicalDevice()
{
    m_Device.Init(m_Instance);
    SA_LOG_INFO(STEXT("Vulkan Device Initialized."));
}

void VulkanRHI::CreateSwapChain()
{
    auto swapchainSupport = m_Device.GetAdapter().QuerySwapchainSupportDetails();
    vk::SurfaceFormatKHR surfaceFormat = ChooseSwapChainFormat(swapchainSupport.formats);
    vk::PresentModeKHR presentMode = ChooseSwapPresentMode(swapchainSupport.presentModes);
    vk::Extent2D extent = ChooseSwapExtent(swapchainSupport.capabilities);

    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
    {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo = {
        .surface = m_Instance.GetSurface(),
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
    };

    QueueFamilyIndices indices = m_Device.GetAdapter().GetQueueFamilyIndices();
    std::array<uint32_t, 2> queueFamilyIndices = { indices.graphics.value(), indices.present.value() };
    if (indices.graphics.value() != indices.present.value())
    {
        createInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
            .setQueueFamilyIndices(queueFamilyIndices);
    } else
    {
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive)
            .setQueueFamilyIndices(nullptr);
    }

    createInfo.setPreTransform(swapchainSupport.capabilities.currentTransform)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setPresentMode(presentMode)
        .setClipped(SA_RHI_TRUE)
        .setOldSwapchain(SA_RHI_NULL);

    Utils::VerifyResult(m_Device->createSwapchainKHR(createInfo, nullptr), STEXT("Failed to create swapchain!"), &m_SwapChain);
    Utils::VerifyResult(m_Device->getSwapchainImagesKHR(m_SwapChain), STEXT("Failed to get swapchain images!"), &m_SwapChainImages);

    m_SwapChainImageFormat = surfaceFormat.format;
    m_SwapChainExtent = extent;

    SA_LOG_INFO(STEXT("Create SwapChain, Complete."));
}
void VulkanRHI::CreateImageViews()
{
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
        Utils::VerifyResult(m_Device->createImageView(createInfo, nullptr),
                            std::format(STEXT("Failed to create ImageView[{}]!"), i), &m_SwapChainImageViews[i]);
    }
    SA_LOG_INFO(STEXT("Create Image Views, Complete."));
}

void VulkanRHI::CreateDescriptorSetLayout()
{
    vk::DescriptorSetLayoutBinding descLayoutBinding = {
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .pImmutableSamplers = nullptr,
    };
    vk::DescriptorSetLayoutCreateInfo createInfo = {
        .bindingCount = 1,
        .pBindings = &descLayoutBinding,
    };
    Utils::VerifyResult(m_Device->createDescriptorSetLayout(createInfo), STEXT("Failed to create descriptor set layout!"), &m_DescriptorSetLayout);
}

void VulkanRHI::CreateGraphicsPipeline()
{
    auto&& fs = FileSystem::GetInstance();
    auto vertShaderBinary = fs.ReadSpirvShaderBinary(ENGINE_PATH("Engine/Shaders/SPIR-V/vert.spv"));
    auto vertShaderModule = CreateShaderModule(vertShaderBinary);
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = vertShaderModule,
        .pName = "main",
    };
    auto fragShaderBinary = fs.ReadSpirvShaderBinary(ENGINE_PATH("Engine/Shaders/SPIR-V/frag.spv"));
    auto fragShaderModule = CreateShaderModule(fragShaderBinary);
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
        .primitiveRestartEnable = SA_RHI_FALSE,
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
        .depthClampEnable = SA_RHI_FALSE,
        .rasterizerDiscardEnable = SA_RHI_FALSE,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eCounterClockwise,
        .depthBiasEnable = SA_RHI_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f,
    };

    vk::PipelineMultisampleStateCreateInfo multisampling = {
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = SA_RHI_FALSE,
        .minSampleShading = 1,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = SA_RHI_FALSE,
        .alphaToOneEnable = SA_RHI_FALSE,
    };

    vk::PipelineColorBlendAttachmentState colorBlendAttachment =
    {
        .blendEnable = SA_RHI_FALSE,
        .srcColorBlendFactor = vk::BlendFactor::eOne,
        .dstColorBlendFactor = vk::BlendFactor::eZero,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
    };

    vk::PipelineColorBlendStateCreateInfo colorBlending = {
        .logicOpEnable = SA_RHI_FALSE,
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
        .setLayoutCount = 1,
        .pSetLayouts = &m_DescriptorSetLayout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };

    Utils::VerifyResult(m_Device->createPipelineLayout(pipelineLayoutInfo), STEXT("Failed to create pipeline layout!"), &m_PipelineLayout);

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
        .basePipelineHandle = SA_RHI_NULL,
        .basePipelineIndex = -1,
    };

    Utils::VerifyResult(m_Device->createGraphicsPipeline(SA_RHI_NULL, graphicsPipelineInfo),
                        STEXT("Failed to create graphics pipeline!"), &m_GraphicsPipeline);

    m_Device->destroyShaderModule(vertShaderModule);
    m_Device->destroyShaderModule(fragShaderModule);
    SA_LOG_INFO(STEXT("Create Graphics Pipeline, Complete."));
}
void VulkanRHI::CreateRenderPass()
{
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

    Utils::VerifyResult(m_Device->createRenderPass(renderPassInfo, nullptr), STEXT("Failed to create render pass!"), &m_RenderPass);
    SA_LOG_INFO(STEXT("Create Render Pass, Complete."))
}
void VulkanRHI::CreateFramebuffers()
{
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

        Utils::VerifyResult(m_Device->createFramebuffer(framebufferInfo, nullptr),
                            std::format(STEXT("Failed to create framebuffer[{}]!"), i), &m_SwapChainFramebuffers[i]);
    }
    SA_LOG_INFO(STEXT("Create Framebuffers, Complete."))
}
void VulkanRHI::CreateCommandPool()
{
    auto& queueFamilyIndices = m_Device.GetAdapter().GetQueueFamilyIndices();

    vk::CommandPoolCreateInfo createInfo = {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = queueFamilyIndices.graphics.value(),
    };

    Utils::VerifyResult(m_Device->createCommandPool(createInfo, nullptr), STEXT("Failed to create command pool!"), &m_CommandPool);
    SA_LOG_INFO(STEXT("Create Command Pool, Complete."))
}
void VulkanRHI::CreateCommandBuffers()
{
    m_CommandBuffers.resize(m_SwapChainFramebuffers.size());

    vk::CommandBufferAllocateInfo allocInfo = {
        .commandPool = m_CommandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size()),
    };
    Utils::VerifyResult(m_Device->allocateCommandBuffers(allocInfo), STEXT("Failed to allocate command buffers!"), &m_CommandBuffers);
    SA_LOG_INFO(STEXT("Create Command Buffers, Complete."))
}

void VulkanRHI::CreateVertexBuffer(ArrayIn<Vertex> triangleVertices)
{

    vk::DeviceSize bufferSize = sizeof(SDecayOf(triangleVertices)::value_type) * triangleVertices.size();
    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;

    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                 &stagingBuffer, &stagingBufferMemory);

    void* data;
    Utils::VerifyResult(m_Device->mapMemory(stagingBufferMemory, 0, bufferSize, {}), STEXT("Failed to map vertex buffer memory!"), &data);
    memcpy(data, triangleVertices.data(), (size_t)bufferSize);
    m_Device->unmapMemory(stagingBufferMemory);

    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                 vk::MemoryPropertyFlagBits::eDeviceLocal,
                 &m_VertexBuffer, &m_VertexBufferMemory);

    Utils::CopyBuffer(this, stagingBuffer, m_VertexBuffer, bufferSize);

    m_Device->destroyBuffer(stagingBuffer);
    m_Device->freeMemory(stagingBufferMemory);
}

void VulkanRHI::CreateIndexBuffer(ArrayIn<uint16_t> triangleIndices)
{
    vk::DeviceSize bufferSize = sizeof(SDecayOf(triangleIndices)::value_type) * triangleIndices.size();
    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;

    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                 &stagingBuffer, &stagingBufferMemory);

    void* data;
    Utils::VerifyResult(m_Device->mapMemory(stagingBufferMemory, 0, bufferSize, {}), STEXT("Failed to map index buffer memory!"), &data);
    memcpy(data, triangleIndices.data(), (size_t)bufferSize);
    m_Device->unmapMemory(stagingBufferMemory);

    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                 vk::MemoryPropertyFlagBits::eDeviceLocal,
                 &m_IndexBuffer, &m_IndexBufferMemory);

    Utils::CopyBuffer(this, stagingBuffer, m_IndexBuffer, bufferSize);

    m_Device->destroyBuffer(stagingBuffer);
    m_Device->freeMemory(stagingBufferMemory);
}

void VulkanRHI::CreateUniformBuffer()
{
    vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
    size_t bufferCount = m_SwapChainImages.size();
    m_UniformBuffers.resize(bufferCount);
    m_UniformBuffersMemory.resize(bufferCount);

    for (size_t i = 0; i < bufferCount; i++)
    {
        CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                     &m_UniformBuffers[i], &m_UniformBuffersMemory[i]);
    }
}

void VulkanRHI::CreateDescriptorPool()
{
    vk::DescriptorPoolSize poolSize = {
        .descriptorCount = static_cast<uint32_t>(m_SwapChainImages.size()),
    };
    vk::DescriptorPoolCreateInfo poolInfo = {
        .maxSets = static_cast<uint32_t>(m_SwapChainImages.size()),
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize,
    };
    Utils::VerifyResult(m_Device->createDescriptorPool(poolInfo), STEXT("Failed to create descriptor pool!"), &m_DescriptorPool);
}

void VulkanRHI::CreateDescriptorSets()
{
    std::vector<vk::DescriptorSetLayout> layouts(m_SwapChainImages.size(), m_DescriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo = {
        .descriptorPool = m_DescriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(m_SwapChainImages.size()),
        .pSetLayouts = layouts.data(),
    };
    m_DescriptorSets.resize(m_SwapChainImages.size());
    Utils::VerifyResult(m_Device->allocateDescriptorSets(allocInfo), STEXT("Failed to alloc descriptor set!"), &m_DescriptorSets);
    for (size_t i = 0; i < m_SwapChainImages.size(); i++)
    {
        vk::DescriptorBufferInfo bufferInfo = {
            .buffer = m_UniformBuffers[i],
            .offset = 0,
            .range = sizeof(UniformBufferObject),
        };
        vk::WriteDescriptorSet descriptorWrite = {
            .dstSet = m_DescriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .pImageInfo = SA_RHI_NULL,
            .pBufferInfo = &bufferInfo,
            .pTexelBufferView = SA_RHI_NULL,
        };
        m_Device->updateDescriptorSets(descriptorWrite, nullptr);
    }
    SA_LOG_INFO(STEXT("Create Descriptor Sets, Complete."))
}

void VulkanRHI::CreateSyncObjects()
{
    m_ImageAvailableSemaphores.resize(m_MaxFrameInFlight);
    m_RenderFinishedSemaphores.resize(m_MaxFrameInFlight);
    m_InFlightFences.resize(m_MaxFrameInFlight);

    vk::SemaphoreCreateInfo semaphoreInfo = {};
    vk::FenceCreateInfo fenceInfo = {
        .flags = vk::FenceCreateFlagBits::eSignaled,
    };

    for (size_t i = 0; i < m_MaxFrameInFlight; i++)
    {
        Utils::VerifyResult(m_Device->createSemaphore(semaphoreInfo, nullptr),
                            STEXT("Failed to create synchronization objects for a frame!"), &m_ImageAvailableSemaphores[i]);

        Utils::VerifyResult(m_Device->createSemaphore(semaphoreInfo, nullptr),
                            STEXT("Failed to create synchronization objects for a frame!"), &m_RenderFinishedSemaphores[i]);

        Utils::VerifyResult(m_Device->createFence(fenceInfo, nullptr),
                            STEXT("Failed to create synchronization objects for a frame!"), &m_InFlightFences[i]);
    }
    SA_LOG_INFO(STEXT("Create Semaphores, Complete."));
}

void VulkanRHI::ReCreateSwapChain()
{
    SharedHandle windowSys = g_GlobalContext.windowSys;
    auto [width, height] = windowSys->GetFramebufferSize();
    while (width == 0 || height == 0)
    {
        std::tie(width, height) = windowSys->GetFramebufferSize();
        windowSys->WaitEvents();
    }
    m_Device->waitIdle();

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
        m_Device->destroyFramebuffer(framebuffer);
    }
    m_Device->freeCommandBuffers(m_CommandPool, m_CommandBuffers);
    m_Device->destroyPipeline(m_GraphicsPipeline);
    m_Device->destroyPipelineLayout(m_PipelineLayout);
    m_Device->destroyRenderPass(m_RenderPass);
    for (auto&& imageView : m_SwapChainImageViews)
    {
        m_Device->destroyImageView(imageView);
    }
    m_Device->destroySwapchainKHR(m_SwapChain);
}

void VulkanRHI::RecordCommandBuffer(ArrayIn<vk::CommandBuffer> cmds, uint32_t idx)
{
    vk::CommandBufferBeginInfo cmdBeginInfo = {
        .flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse,
        .pInheritanceInfo = nullptr,
    };

    Utils::VerifyResult(cmds[idx].begin(cmdBeginInfo),
                        [&, this](auto result) {
                            if (result != vk::Result::eSuccess)
                            {
                                SA_LOG_ERROR(STEXT("Failed to begin recording command buffer!"));
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
                                std::array<vk::DeviceSize, 1>   offsets = { 0 };
                                cmds[idx].bindVertexBuffers(0, 1, vertexBuffers.data(), offsets.data());
                                cmds[idx].bindIndexBuffer(m_IndexBuffer, 0, vk::IndexType::eUint16);

                                cmds[idx].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PipelineLayout, 0, m_DescriptorSets[idx], nullptr);

                                cmds[idx].drawIndexed(static_cast<uint32_t>(g_TriangleIndices.size()), 1, 0, 0, 0);
                                cmds[idx].endRenderPass();

                                Utils::VerifyResult(cmds[idx].end(), STEXT("Failed to end recording command buffer!"));
                            }
                        });
}

void VulkanRHI::DrawFrame()
{
    auto waitForFencesResult = m_Device->waitForFences(m_InFlightFences[m_CurrentFrame], SA_RHI_TRUE, std::numeric_limits<uint64_t>::max());

    uint32_t imageIndex;
    Utils::VerifyResult(m_Device->acquireNextImageKHR(m_SwapChain, std::numeric_limits<uint64_t>::max(), 
                                                     m_ImageAvailableSemaphores[m_CurrentFrame], SA_RHI_NULL, &imageIndex),
                        [this](auto result) {
                            if (result == vk::Result::eErrorOutOfDateKHR)
                            {
                                ReCreateSwapChain();
                                return;
                            } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
                            {
                                SA_LOG_ERROR(STEXT("Failed to acquire swap chain image!"));
                            }
                        });

    UpdateUniformBuffer(imageIndex);

    m_Device->resetFences(m_InFlightFences[m_CurrentFrame]);

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

    Utils::VerifyResult(m_Device.GetQueue(ERHIQueueType::Graphics).submit(submitInfo, m_InFlightFences[m_CurrentFrame]), STEXT("Failed to submit draw command buffer!"));

    vk::PresentInfoKHR presentInfo = {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_RenderFinishedSemaphores[m_CurrentFrame],
        .swapchainCount = 1,
        .pSwapchains = &m_SwapChain,
        .pImageIndices = &imageIndex,
    };

    Utils::VerifyResult(m_Device.GetQueue(ERHIQueueType::Present).presentKHR(presentInfo),
                        [this](auto result) {
                            SharedHandle windowSys = g_GlobalContext.windowSys;
                            if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || windowSys->IsFramebufferResized())
                            {
                                windowSys->SetFramebufferResizedToDefault();
                                ReCreateSwapChain();
                            } else if (result != vk::Result::eSuccess)
                            {
                                SA_LOG_ERROR(STEXT("Failed to present swap chain image!"));
                            }
                        });

    //vkQueueWaitIdle(m_PresentQueue);
    m_CurrentFrame = (m_CurrentFrame + 1) % m_MaxFrameInFlight;
}


// ==============================================
// Tool Functions
// ==============================================
vk::SurfaceFormatKHR VulkanRHI::ChooseSwapChainFormat(ArrayIn<vk::SurfaceFormatKHR> availableFormats)
{
    if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined)
    {
        return { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
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
vk::PresentModeKHR VulkanRHI::ChooseSwapPresentMode(ArrayIn<vk::PresentModeKHR> availablePresentModes)
{
    auto bestMode = vk::PresentModeKHR::eFifo;
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox)
        {
            return availablePresentMode;
        } else
        {
            bestMode = vk::PresentModeKHR::eImmediate;
        }
    }
    return bestMode;
}
vk::Extent2D VulkanRHI::ChooseSwapExtent(In<vk::SurfaceCapabilitiesKHR> capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    } else
    {
        int actualWidth, actualHeight;
        glfwGetFramebufferSize(m_WindowHandle, &actualWidth, &actualHeight);
        vk::Extent2D actualExtent = { static_cast<uint32_t>(actualWidth), static_cast<uint32_t>(actualHeight) };
        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }
}
vk::ShaderModule VulkanRHI::CreateShaderModule(ArrayIn<char> code)
{
    vk::ShaderModule shaderModule;

    vk::ShaderModuleCreateInfo createInfo = {
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t*>(code.data()),
    };

    Utils::VerifyResult(m_Device->createShaderModule(createInfo, nullptr), STEXT("Failed to create shader module!"), &shaderModule);

    return shaderModule;
}
uint32_t VulkanRHI::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties props = m_Device.GetAdapter()->getMemoryProperties();
    for (uint32_t i = 0; i < props.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (props.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    SA_LOG_ERROR(STEXT("Failed to find suitable memory type!"));
    return 0;
}
void VulkanRHI::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, Out<vk::Buffer> buffer, Out<vk::DeviceMemory> bufferMemory)
{
    vk::BufferCreateInfo bufferInfo = {
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
    };
    Utils::VerifyResult(m_Device->createBuffer(bufferInfo), STEXT("Failed to create buffer!"), buffer);

    vk::MemoryRequirements memRequirements;
    m_Device->getBufferMemoryRequirements(*buffer, &memRequirements);
    vk::MemoryAllocateInfo allocInfo = {
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties),
    };
    Utils::VerifyResult(m_Device->allocateMemory(allocInfo), STEXT("Failed to allocate vertex buffer memory!"), bufferMemory);
    Utils::VerifyResult(m_Device->bindBufferMemory(*buffer, *bufferMemory, 0), STEXT("Failed to bind vertex buffer memory!"));
}
void VulkanRHI::UpdateUniformBuffer(uint32_t idx)
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float>(currentTime - startTime).count();

    UniformBufferObject ubo = {};
    ubo.modelMatrix = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.viewMatrix = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.projectMatrix = glm::perspective(glm::radians(45.0f), static_cast<float>(m_SwapChainExtent.width) / m_SwapChainExtent.height, 0.1f, 10.0f);
    ubo.projectMatrix[1][1] *= -1;  // for vulkan

    void* data;
    Utils::VerifyResult(m_Device->mapMemory(m_UniformBuffersMemory[idx], 0, sizeof(ubo), {}), STEXT("Failed to map index buffer memory!"), &data);
    memcpy(data, &ubo, sizeof(ubo));
    m_Device->unmapMemory(m_UniformBuffersMemory[idx]);
}

}
