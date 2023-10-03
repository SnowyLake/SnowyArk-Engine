#include "VulkanRHI.h"
#include "Engine/Source/Runtime/Core/Log/Logger.h"
#include "Engine/Source/Runtime/Function/Global/GlobalContext.h"
#include "Engine/Source/Runtime/Function/Window/WindowSystem.h"
#include "Engine/Source/Runtime/Resource/AssetManager.h"

#include <set>

namespace Snowy::Ark
{
using Utils = VulkanUtils;

VulkanRHI::VulkanRHI()
{
    //m_VertexBuffer = MakeUnique<VulkanBuffer>();
    //m_IndexBuffer = MakeUnique<VulkanBuffer>();

    //m_DepthAttachment = MakeUnique<VulkanTexture>();
    //m_Texture = MakeUnique<VulkanTexture>();
}

void VulkanRHI::Init(In<RHIConfig> config)
{
    Init_Internal(config);
    SA_LOG_INFO("Vulkan Context Initialized.");
    SA_LOG_INFO("========================================================");

    PostInit_Internal();
}

void VulkanRHI::Run()
{
    DrawFrame();
}

void VulkanRHI::Init_Internal(In<RHIConfig> config)
{
    SetWindowHandle(config.windowHandle);

    CreateInstance(&m_Instance, config);

    m_Device = m_Instance.CreateDevice();
    m_Swapchain = m_Device.CreateSwapchain();
}

void VulkanRHI::PostInit_Internal()
{
    CreateCommandPool();

    CreateDepthAttachment();

    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateFramebuffers();

    CreateVertexBuffer(g_TriangleVertices);
    CreateIndexBuffer(g_TriangleIndices);
    CreateUniformBuffer();
    
    CreateSampledTexture(SA_ENGINE_PATH("Engine/Assets/Texture/texture.jpg"));

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

    m_VertexBuffer->Destroy();
    m_IndexBuffer->Destroy();
    for (size_t i = 0; i < m_Swapchain.Count(); i++)
    {
        m_UniformBuffers[i]->Destroy();
    }

    for (size_t i = 0; i < m_Instance.GetFrameCountInFlight(); i++)
    {
        m_Device->destroySemaphore(m_ImageAvailableSemaphores[i]);
        m_Device->destroySemaphore(m_RenderFinishedSemaphores[i]);
        m_Device->destroyFence(m_InFlightFences[i]);
    }

    m_Texture->Destroy();

    m_Device->destroyCommandPool(m_CommandPool);

    m_Device.Destroy();
    m_Instance.Destroy();

    SA_LOG_INFO("Vulkan Context Destoryed.");
}

vk::CommandBuffer VulkanRHI::BeginSingleTimeCommandBuffer()
{
    vk::CommandBufferAllocateInfo allocInfo = {
        .commandPool = m_CommandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };
    vk::CommandBuffer cmd;
    Utils::VerifyResult(m_Device->allocateCommandBuffers(allocInfo),
                        [&](const auto& result) {
                            if (result.result != vk::Result::eSuccess)
                            {
                                SA_LOG_ERROR("Failed to allocate single time command!");
                            } else
                            {
                                cmd = result.value[0];
                            }
                        });
    vk::CommandBufferBeginInfo beginInfo = {
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    };
    Utils::VerifyResult(cmd.begin(beginInfo), STEXT("Failed to begin recording single time command!"));
    return cmd;
}

void VulkanRHI::EndSingleTimeCommandBuffer(vk::CommandBuffer cmd)
{
    cmd.end();
    vk::SubmitInfo info = {
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };
    auto& queue = m_Device.Queue(ERHIQueue::Graphics);
    Utils::VerifyResult(queue.submit(info), STEXT("Failed to submit single time command!"));
    queue.waitIdle();
    m_Device->freeCommandBuffers(m_CommandPool, cmd);
}

void VulkanRHI::RecreateSwapchain()
{
    SharedHandle windowSys = g_RuntimeContext.windowSys;
    auto [width, height] = windowSys->GetFramebufferSize();
    while (width == 0 || height == 0)
    {
        std::tie(width, height) = windowSys->GetFramebufferSize();
        windowSys->WaitEvents();
    }
    m_Device->waitIdle();

    CleanupSwapChain();

    m_Swapchain.Recreate();

    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateDepthAttachment();
    CreateFramebuffers();
    CreateCommandBuffers();
    SA_LOG_INFO("Recreate SwapChain, Complete.");
}

void VulkanRHI::CleanupSwapChain()
{
    m_DepthAttachment->Destroy();
    for (auto&& framebuffer : m_SwapchainFramebuffers)
    {
        m_Device->destroyFramebuffer(framebuffer);
    }
    m_Device->freeCommandBuffers(m_CommandPool, m_CommandBuffers);
    m_Device->destroyPipeline(m_GraphicsPipeline);
    m_Device->destroyPipelineLayout(m_PipelineLayout);
    m_Device->destroyRenderPass(m_RenderPass);
    m_Swapchain.Destory();
}

void VulkanRHI::CreateDescriptorSetLayout()
{
    vk::DescriptorSetLayoutBinding uboLayoutBinding = {
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .pImmutableSamplers = SA_RHI_NULL,
    };
    vk::DescriptorSetLayoutBinding samplerLayoutBinding = {
        .binding = 1,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eFragment,
        .pImmutableSamplers = SA_RHI_NULL,
    };
    std::array bindings = { uboLayoutBinding, samplerLayoutBinding };
    vk::DescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.setBindings(bindings);
    Utils::VerifyResult(m_Device->createDescriptorSetLayout(createInfo), STEXT("Failed to create descriptor set layout!"), &m_DescriptorSetLayout);
}

void VulkanRHI::CreateGraphicsPipeline()
{
    SharedHandle assetMgr = g_RuntimeContext.assetMgr;
    auto vertShaderBinary = assetMgr->LoadSpirvShaderBinary(SA_ENGINE_PATH("Engine/Shaders/SPIR-V/vert.spv"));
    auto vertShaderModule = m_Device.CreateShaderModule(vertShaderBinary);
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = vertShaderModule,
        .pName = "main",
    };
    auto fragShaderBinary = assetMgr->LoadSpirvShaderBinary(SA_ENGINE_PATH("Engine/Shaders/SPIR-V/frag.spv"));
    auto fragShaderModule = m_Device.CreateShaderModule(fragShaderBinary);
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = fragShaderModule,
        .pName = "main",
    };

    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

    auto bindingDescription = SimpleVertex::GetBindingDescription();
    auto attributeDescriptions = SimpleVertex::GetAttributeDescriptions();
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = SA_VK_NUM(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data(),
    };

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = SA_RHI_FALSE,
    };

    vk::Viewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(m_Swapchain.Extent().width),
        .height = static_cast<float>(m_Swapchain.Extent().height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    vk::Rect2D scissor = {
        .offset = {0, 0},
        .extent = m_Swapchain.Extent(),
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

    vk::PipelineDepthStencilStateCreateInfo depthStencil = {
        .depthTestEnable = SA_RHI_TRUE,
        .depthWriteEnable = SA_RHI_TRUE,
        .depthCompareOp = vk::CompareOp::eLess,
        .depthBoundsTestEnable = SA_RHI_FALSE,
        .stencilTestEnable = SA_RHI_FALSE,
        .front = {},
        .back = {},
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f,
    };

    vk::PipelineColorBlendAttachmentState colorBlendAttachment = {
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
        .dynamicStateCount = SA_VK_NUM(dynamicStates.size()),
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
        .stageCount = SA_VK_NUM(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colorBlending,
        .pDynamicState = nullptr,
        .layout = m_PipelineLayout,
        .renderPass = m_RenderPass,
        .subpass = 0,
        .basePipelineHandle = SA_RHI_NULL,
        .basePipelineIndex = -1,
    };

    Utils::VerifyResult(m_Device->createGraphicsPipeline(SA_RHI_NULL, graphicsPipelineInfo), STEXT("Failed to create graphics pipeline!"), &m_GraphicsPipeline);

    m_Device->destroyShaderModule(vertShaderModule);
    m_Device->destroyShaderModule(fragShaderModule);
    SA_LOG_INFO("Create Graphics Pipeline, Complete.");
}
void VulkanRHI::CreateRenderPass()
{
    vk::AttachmentDescription colorAttachmentDesc = {
        .flags = vk::AttachmentDescriptionFlags{},
        .format = m_Swapchain.Format(),
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

    vk::AttachmentDescription depthAttachmentDesc = {
        .flags = vk::AttachmentDescriptionFlags{},
        .format = GetDepthFormat(),
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
    };
    vk::AttachmentReference depthAttachmentRef = {
        .attachment = 1,
        .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
    };

    vk::SubpassDescription subpassDesc = {
        .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pDepthStencilAttachment = &depthAttachmentRef,
    };

    vk::SubpassDependency subpassDependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .srcAccessMask = vk::AccessFlagBits::eNone,
        .dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
    };

    std::array attachments = { colorAttachmentDesc, depthAttachmentDesc };

    vk::RenderPassCreateInfo renderPassInfo = {
        .attachmentCount = SA_VK_NUM(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpassDesc,
        .dependencyCount = 1,
        .pDependencies = &subpassDependency,
    };

    Utils::VerifyResult(m_Device->createRenderPass(renderPassInfo, nullptr), STEXT("Failed to create render pass!"), &m_RenderPass);
    SA_LOG_INFO("Create Render Pass, Complete.");
}

void VulkanRHI::CreateFramebuffers()
{
    m_SwapchainFramebuffers.resize(m_Swapchain.Count());
    for (size_t i = 0; i < m_Swapchain.Count(); i++)
    {
        std::array attachments = { m_Swapchain.View(i), m_DepthAttachment->View() };

        vk::FramebufferCreateInfo framebufferInfo = {
            .renderPass = m_RenderPass,
            .attachmentCount = SA_VK_NUM(attachments.size()),
            .pAttachments = attachments.data(),
            .width = m_Swapchain.Extent().width,
            .height = m_Swapchain.Extent().height,
            .layers = 1,
        };

        Utils::VerifyResult(m_Device->createFramebuffer(framebufferInfo, nullptr),
                            std::format(STEXT("Failed to create framebuffer[{}]!"), i), &m_SwapchainFramebuffers[i]);
    }
    SA_LOG_INFO("Create Framebuffers, Complete.");
}

void VulkanRHI::CreateCommandPool()
{
    auto& queueFamilyIndices = m_Device.Adapter().GetQueueFamilyIndices();

    vk::CommandPoolCreateInfo createInfo = {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = queueFamilyIndices.graphics.value(),
    };

    Utils::VerifyResult(m_Device->createCommandPool(createInfo, nullptr), STEXT("Failed to create command pool!"), &m_CommandPool);
    SA_LOG_INFO("Create Command Pool, Complete.");
}

void VulkanRHI::CreateCommandBuffers()
{
    m_CommandBuffers.resize(m_SwapchainFramebuffers.size());

    vk::CommandBufferAllocateInfo allocInfo = {
        .commandPool = m_CommandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size()),
    };
    Utils::VerifyResult(m_Device->allocateCommandBuffers(allocInfo), STEXT("Failed to allocate command buffers!"), &m_CommandBuffers);
    SA_LOG_INFO("Create Command Buffers, Complete.");
}

void VulkanRHI::CreateVertexBuffer(ArrayIn<SimpleVertex> triangleVertices)
{
    vk::DeviceSize bufferSize = sizeof(decltype(triangleVertices)::value_type) * triangleVertices.size();

    auto stagingBuffer = m_Device.CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    void* data;
    Utils::VerifyResult(m_Device->mapMemory(stagingBuffer->Memory(), 0, bufferSize, {}), STEXT("Failed to map vertex buffer memory!"), &data);
    memcpy(data, triangleVertices.data(), (size_t)bufferSize);
    m_Device->unmapMemory(stagingBuffer->Memory());

    m_VertexBuffer = m_Device.CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

    CopyBuffer(*stagingBuffer, *m_VertexBuffer, bufferSize);

    stagingBuffer->Destroy();
}

void VulkanRHI::CreateIndexBuffer(ArrayIn<uint16_t> triangleIndices)
{
    vk::DeviceSize bufferSize = sizeof(decltype(triangleIndices)::value_type) * triangleIndices.size();

    auto stagingBuffer = m_Device.CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    void* data;
    Utils::VerifyResult(m_Device->mapMemory(stagingBuffer->Memory(), 0, bufferSize, {}), STEXT("Failed to map index buffer memory!"), &data);
    memcpy(data, triangleIndices.data(), (size_t)bufferSize);
    m_Device->unmapMemory(stagingBuffer->Memory());

    m_IndexBuffer = m_Device.CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

    CopyBuffer(*stagingBuffer, *m_IndexBuffer, bufferSize);

    stagingBuffer->Destroy();
}

void VulkanRHI::CreateUniformBuffer()
{
    vk::DeviceSize bufferSize = sizeof(SACommonMatrices);
    size_t bufferCount = m_Swapchain.Count();
    m_UniformBuffers.resize(bufferCount);

    for (size_t i = 0; i < bufferCount; i++)
    {
        m_UniformBuffers[i] = m_Device.CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    }
}

void VulkanRHI::CreateDepthAttachment()
{
    auto depthFormat = GetDepthFormat();
    TextureData textureData = {
        .pixels = SA_RHI_NULL,
        .width = static_cast<int>(m_Swapchain.Extent().width),
        .height = static_cast<int>(m_Swapchain.Extent().height),
        .channel = 2,
    };
    VulkanTextureParams textureParams = {
        .type = vk::ImageType::e2D,
        .format = depthFormat,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
        .memoryProps = vk::MemoryPropertyFlagBits::eDeviceLocal,

        .viewType = vk::ImageViewType::e2D,
        .aspectMask = vk::ImageAspectFlagBits::eDepth,
    };
    m_DepthAttachment = m_Device.CreateTexture(textureData, textureParams);
    m_DepthAttachment->TransitionLayout(depthFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
}

void VulkanRHI::CreateSampledTexture(std::filesystem::path path)
{
    auto textureData = g_RuntimeContext.assetMgr->LoadTexture(path);
    vk::DeviceSize texSize = textureData->width * textureData->height * 4;

    auto stagingBuffer = m_Device.CreateBuffer(texSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    void* data = m_Device->mapMemory(stagingBuffer->Memory(), 0, texSize, {}).value;
    memcpy(data, textureData->pixels, static_cast<size_t>(texSize));
    m_Device->unmapMemory(stagingBuffer->Memory());

    VulkanTextureParams textureParams = {
        .type = vk::ImageType::e2D,
        .format = vk::Format::eR8G8B8A8Unorm,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        .memoryProps = vk::MemoryPropertyFlagBits::eDeviceLocal,

        .viewType = vk::ImageViewType::e2D,
        .aspectMask = vk::ImageAspectFlagBits::eColor,
    };
    m_Texture = m_Device.CreateTexture(*textureData, textureParams);

    m_Texture->TransitionLayout(vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    CopyBufferToImage(*stagingBuffer, *m_Texture, SA_VK_NUM(textureData->width), SA_VK_NUM(textureData->height));
    m_Texture->TransitionLayout(vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    stagingBuffer->Destroy();
}

void VulkanRHI::CreateDescriptorPool()
{
    std::array<vk::DescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0] = {
        .type = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = m_Swapchain.Count(),
    };
    poolSizes[1] = {
        .type = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = m_Swapchain.Count(),
    };
    vk::DescriptorPoolCreateInfo poolInfo = {
        .maxSets = m_Swapchain.Count(),
        .poolSizeCount = SA_VK_NUM(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };
    Utils::VerifyResult(m_Device->createDescriptorPool(poolInfo), STEXT("Failed to create descriptor pool!"), &m_DescriptorPool);
}

void VulkanRHI::CreateDescriptorSets()
{
    std::vector<vk::DescriptorSetLayout> layouts(m_Swapchain.Count(), m_DescriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo = {
        .descriptorPool = m_DescriptorPool,
        .descriptorSetCount = m_Swapchain.Count(),
        .pSetLayouts = layouts.data(),
    };
    m_DescriptorSets.resize(m_Swapchain.Count());
    Utils::VerifyResult(m_Device->allocateDescriptorSets(allocInfo), STEXT("Failed to alloc descriptor set!"), &m_DescriptorSets);
    for (size_t i = 0; i < m_Swapchain.Count(); i++)
    {
        vk::DescriptorBufferInfo bufferInfo = {
            .buffer = *m_UniformBuffers[i],
            .offset = 0,
            .range = sizeof(SACommonMatrices),
        };
        vk::DescriptorImageInfo imageInfo = {
            .sampler = m_Texture->Sampler(),
            .imageView = m_Texture->View(),
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
        };

        std::array<vk::WriteDescriptorSet, 2> descriptorWrites = {};
        descriptorWrites[0] = vk::WriteDescriptorSet{
            .dstSet = m_DescriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .pImageInfo = SA_RHI_NULL,
            .pBufferInfo = &bufferInfo,
            .pTexelBufferView = SA_RHI_NULL,
        };
        descriptorWrites[1] = vk::WriteDescriptorSet{
            .dstSet = m_DescriptorSets[i],
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = &imageInfo,
            .pBufferInfo = SA_RHI_NULL,
            .pTexelBufferView = SA_RHI_NULL,
        };
        m_Device->updateDescriptorSets(descriptorWrites, nullptr);
    }
    SA_LOG_INFO("Create Descriptor Sets, Complete.");
}

void VulkanRHI::CreateSyncObjects()
{
    auto frameCount = m_Instance.GetFrameCountInFlight();
    m_ImageAvailableSemaphores.resize(frameCount);
    m_RenderFinishedSemaphores.resize(frameCount);
    m_InFlightFences.resize(frameCount);

    vk::SemaphoreCreateInfo semaphoreInfo = {};
    vk::FenceCreateInfo fenceInfo = {
        .flags = vk::FenceCreateFlagBits::eSignaled,
    };

    for (size_t i = 0; i < frameCount; i++)
    {
        Utils::VerifyResult(m_Device->createSemaphore(semaphoreInfo, nullptr),
                            STEXT("Failed to create synchronization objects for a frame!"), &m_ImageAvailableSemaphores[i]);

        Utils::VerifyResult(m_Device->createSemaphore(semaphoreInfo, nullptr),
                            STEXT("Failed to create synchronization objects for a frame!"), &m_RenderFinishedSemaphores[i]);

        Utils::VerifyResult(m_Device->createFence(fenceInfo, nullptr),
                            STEXT("Failed to create synchronization objects for a frame!"), &m_InFlightFences[i]);
    }
    SA_LOG_INFO("Create Semaphores, Complete.");
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
                                SA_LOG_ERROR("Failed to begin recording command buffer!");
                            } else
                            {
                                std::array clearValues = {
                                    vk::ClearValue{.color = std::array{0.0f, 0.0f, 0.0f, 1.0f} },
                                    vk::ClearValue{.depthStencil = {1.0f, 0} }
                                };
                                vk::RenderPassBeginInfo renderPassBeginInfo = {
                                    .renderPass = m_RenderPass,
                                    .framebuffer = m_SwapchainFramebuffers[idx],
                                    .renderArea = vk::Rect2D {
                                        .offset = {0, 0},
                                        .extent = m_Swapchain.Extent(),
                                    },
                                    .clearValueCount = SA_VK_NUM(clearValues.size()),
                                    .pClearValues = clearValues.data(),
                                };

                                cmds[idx].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
                                cmds[idx].bindPipeline(vk::PipelineBindPoint::eGraphics, m_GraphicsPipeline);

                                std::array<vk::Buffer, 1> vertexBuffers = { *m_VertexBuffer };
                                std::array<vk::DeviceSize, 1>   offsets = { 0 };
                                cmds[idx].bindVertexBuffers(0, 1, vertexBuffers.data(), offsets.data());
                                cmds[idx].bindIndexBuffer(*m_IndexBuffer, 0, vk::IndexType::eUint16);

                                cmds[idx].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PipelineLayout, 0, m_DescriptorSets[idx], nullptr);

                                cmds[idx].drawIndexed(SA_VK_NUM(g_TriangleIndices.size()), 1, 0, 0, 0);
                                cmds[idx].endRenderPass();

                                Utils::VerifyResult(cmds[idx].end(), STEXT("Failed to end recording command buffer!"));
                            }
                        });
}

void VulkanRHI::DrawFrame()
{
    auto waitForFencesResult = m_Device->waitForFences(m_InFlightFences[m_CurrFrameIndex], SA_RHI_TRUE, std::numeric_limits<uint64_t>::max());

    uint32_t imageIdx;
    Utils::VerifyResult(m_Device->acquireNextImageKHR(m_Swapchain, std::numeric_limits<uint64_t>::max(),
                                                      m_ImageAvailableSemaphores[m_CurrFrameIndex], SA_RHI_NULL, &imageIdx),
                        [this](auto result) {
                            if (result == vk::Result::eErrorOutOfDateKHR)
                            {
                                RecreateSwapchain();
                                return;
                            } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
                            {
                                SA_LOG_ERROR("Failed to acquire swap chain image!");
                            }
                        });

    m_Device->resetFences(m_InFlightFences[m_CurrFrameIndex]);

    UpdateUniformBuffer(imageIdx);
    RecordCommandBuffer(m_CommandBuffers, imageIdx);

    std::array<vk::PipelineStageFlags, 1> waitDstStageMask = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

    vk::SubmitInfo submitInfo = {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_ImageAvailableSemaphores[m_CurrFrameIndex],
        .pWaitDstStageMask = waitDstStageMask.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &m_CommandBuffers[imageIdx],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_RenderFinishedSemaphores[m_CurrFrameIndex],
    };

    Utils::VerifyResult(m_Device.Queue(ERHIQueue::Graphics).submit(submitInfo, m_InFlightFences[m_CurrFrameIndex]), STEXT("Failed to submit draw command buffer!"));

    vk::PresentInfoKHR presentInfo = {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_RenderFinishedSemaphores[m_CurrFrameIndex],
        .swapchainCount = 1,
        .pSwapchains = &m_Swapchain.Native(),
        .pImageIndices = &imageIdx,
    };

    Utils::VerifyResult(m_Device.Queue(ERHIQueue::Present).presentKHR(presentInfo),
                        [this](auto result) {
                            SharedHandle windowSys = g_RuntimeContext.windowSys;
                            if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || windowSys->IsFramebufferResized())
                            {
                                windowSys->SetFramebufferResizedToDefault();
                                RecreateSwapchain();
                            } else if (result != vk::Result::eSuccess)
                            {
                                SA_LOG_ERROR("Failed to present swap chain image!");
                            }
                        });

    m_CurrFrameIndex = (m_CurrFrameIndex + 1) % m_Instance.GetFrameCountInFlight();
}


void VulkanRHI::CreateInstance(Out<VulkanInstance> instance, In<RHIConfig> config) noexcept
{
    instance->SetFrameCountInFlight(config.frameCountInFlight);
#ifdef NDEBUG
    instance->EnableValidationLayers() = false;
#else
    instance->EnableValidationLayers() = true & config.vkEnableValidationLayers;
#endif
    instance->ValidationLayers().append_range(config.vkValidationLayers);

    uint32_t glfwExtensionCount = 0;
    const AnsiChar** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    instance->RequiredExtensions().append_range(std::vector<const AnsiChar*>(glfwExtensions, glfwExtensions + glfwExtensionCount));
    if (instance->EnableValidationLayers())
    {
        instance->RequiredExtensions().emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    instance->RequiredDeviceExtensions().append_range(config.vkDeviceExtensions);
    instance->Init(this);
    SA_LOG_INFO("Vulkan Instance Initialized.");
}

// ==============================================
// Tool Functions
// ==============================================

void VulkanRHI::UpdateUniformBuffer(uint32_t idx)
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float>(currentTime - startTime).count();

    SACommonMatrices ubo = {};
    ubo.SA_ObjectToWorld = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.SA_MatrixV = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.SA_MatrixP = glm::perspective(glm::radians(45.0f), static_cast<float>(m_Swapchain.Extent().width) / m_Swapchain.Extent().height, 0.1f, 10.0f);
    ubo.SA_MatrixP[1][1] *= -1;  // for vulkan

    void* data;
    Utils::VerifyResult(m_Device->mapMemory(m_UniformBuffers[idx]->Memory(), 0, sizeof(ubo), {}), STEXT("Failed to map index buffer memory!"), &data);
    memcpy(data, &ubo, sizeof(ubo));
    m_Device->unmapMemory(m_UniformBuffers[idx]->Memory());
}

void VulkanRHI::CopyBuffer(In<VulkanBuffer> srcBuffer, Ref<VulkanBuffer> dstBuffer, vk::DeviceSize size)
{
    auto cmd = BeginSingleTimeCommandBuffer();
    vk::BufferCopy copyRegion = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size,
    };
    cmd.copyBuffer(srcBuffer, dstBuffer, copyRegion);
    EndSingleTimeCommandBuffer(cmd);
}

void VulkanRHI::CopyBufferToImage(In<VulkanBuffer> srcBuffer, Ref<VulkanTexture> dstImage, uint32_t width, uint32_t height)
{
    auto cmd = BeginSingleTimeCommandBuffer();
    vk::BufferImageCopy copyRegion = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = vk::ImageSubresourceLayers {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .imageOffset = {0, 0, 0},
        .imageExtent = {width, height, 1},
    };
    cmd.copyBufferToImage(srcBuffer, dstImage, vk::ImageLayout::eTransferDstOptimal, copyRegion);
    EndSingleTimeCommandBuffer(cmd);
}

vk::Format VulkanRHI::FindSupportedFormat(ArrayIn<vk::Format> formats, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const noexcept
{
    for (auto& format : formats)
    {
        vk::FormatProperties props = m_Device.Adapter()->getFormatProperties(format);
        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)
        {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }
    SA_LOG_ERROR("Failed to find supported format!");
    return vk::Format::eUndefined;
}

vk::Format VulkanRHI::GetDepthFormat() const noexcept
{
    return FindSupportedFormat(Utils::CommonDepthFormats, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}
}
