#include "VulkanRHI.h"
#include "Engine/Source/Runtime/Core/Log/Logger.h"
#include "Engine/Source/Runtime/Function/Global/GlobalContext.h"
#include "Engine/Source/Runtime/Function/Window/WindowSystem.h"
#include "Engine/Source/Runtime/Platform/FileSystem.h"

#include <set>

namespace Snowy::Ark
{
using Utils = VulkanUtils;

void VulkanRHI::Init(In<RHIConfig> config)
{
    Init_Internal(config);
    SA_LOG_INFO(STEXT("Vulkan Context Initialized."));
    SA_LOG_INFO(STEXT("========================================================"));

    PostInit_Internal();
}

void VulkanRHI::Run()
{
    DrawFrame();
}

void VulkanRHI::Init_Internal(In<RHIConfig> config)
{
    m_WindowHandle = config.windowHandle;
    m_MaxFrameCountInFlight = config.maxFrameInFlight;

    CreateInstance(&m_Instance, config);
    m_Instance.CreateDevice(&m_Device);
    m_Device.CreateSwapchain(&m_Swapchain);
}

void VulkanRHI::PostInit_Internal()
{
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

    for (size_t i = 0; i < m_Swapchain.GetImageCount(); i++)
    {
        m_Device->destroyBuffer(m_UniformBuffers[i]);
        m_Device->freeMemory(m_UniformBuffersMemory[i]);
    }

    for (size_t i = 0; i < m_MaxFrameCountInFlight; i++)
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

void VulkanRHI::RecreateSwapchain()
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

    m_Swapchain.Recreate();

    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandBuffers();
    SA_LOG_INFO(STEXT("Recreate SwapChain, Complete."));
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
    m_Swapchain.Destory();
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
    // TODO: Move FileSystem to GlobalContext
    auto&& fs = FileSystem::GetInstance();
    auto vertShaderBinary = fs.ReadSpirvShaderBinary(ENGINE_PATH("Engine/Shaders/SPIR-V/vert.spv"));
    auto vertShaderModule = m_Device.CreateShaderModule(vertShaderBinary);
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = vertShaderModule,
        .pName = "main",
    };
    auto fragShaderBinary = fs.ReadSpirvShaderBinary(ENGINE_PATH("Engine/Shaders/SPIR-V/frag.spv"));
    auto fragShaderModule = m_Device.CreateShaderModule(fragShaderBinary);
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = fragShaderModule,
        .pName = "main",
    };

    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

    auto bindingDescription = TempVertex::GetBindingDescription();
    auto attributeDescriptions = TempVertex::GetAttributeDescriptions();
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
        .width = static_cast<float>(m_Swapchain.GetExtent().width),
        .height = static_cast<float>(m_Swapchain.GetExtent().height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    vk::Rect2D scissor = {
        .offset = {0, 0},
        .extent = m_Swapchain.GetExtent(),
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
        .dynamicStateCount = Utils::CastNumType(dynamicStates.size()),
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
        .stageCount = Utils::CastNumType(shaderStages.size()),
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

    Utils::VerifyResult(m_Device->createGraphicsPipeline(SA_RHI_NULL, graphicsPipelineInfo), STEXT("Failed to create graphics pipeline!"), &m_GraphicsPipeline);

    m_Device->destroyShaderModule(vertShaderModule);
    m_Device->destroyShaderModule(fragShaderModule);
    SA_LOG_INFO(STEXT("Create Graphics Pipeline, Complete."));
}
void VulkanRHI::CreateRenderPass()
{
    vk::AttachmentDescription colorAttachmentDesc = {
        .flags = vk::AttachmentDescriptionFlags{},
        .format = m_Swapchain.GetImageFormat(),
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
    m_SwapChainFramebuffers.resize(m_Swapchain.GetImageCount());
    for (size_t i = 0; i < m_Swapchain.GetImageCount(); i++)
    {
        std::array<vk::ImageView, 1> attachments{ m_Swapchain.GetImageView(i) };

        vk::FramebufferCreateInfo framebufferInfo = {
            .renderPass = m_RenderPass,
            .attachmentCount = Utils::CastNumType(attachments.size()),
            .pAttachments = attachments.data(),
            .width = m_Swapchain.GetExtent().width,
            .height = m_Swapchain.GetExtent().height,
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

void VulkanRHI::CreateVertexBuffer(ArrayIn<TempVertex> triangleVertices)
{

    vk::DeviceSize bufferSize = sizeof(SDecayOf(triangleVertices)::value_type) * triangleVertices.size();

    auto [stagingBuffer, stagingBufferMemory] 
        = m_Device.CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data;
    Utils::VerifyResult(m_Device->mapMemory(stagingBufferMemory, 0, bufferSize, {}), STEXT("Failed to map vertex buffer memory!"), &data);
    memcpy(data, triangleVertices.data(), (size_t)bufferSize);
    m_Device->unmapMemory(stagingBufferMemory);

    std::tie(m_VertexBuffer, m_VertexBufferMemory) 
        = m_Device.CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, 
                                vk::MemoryPropertyFlagBits::eDeviceLocal);

    Utils::CopyBuffer(this, stagingBuffer, m_VertexBuffer, bufferSize);

    m_Device->destroyBuffer(stagingBuffer);
    m_Device->freeMemory(stagingBufferMemory);
}

void VulkanRHI::CreateIndexBuffer(ArrayIn<uint16_t> triangleIndices)
{
    vk::DeviceSize bufferSize = sizeof(SDecayOf(triangleIndices)::value_type) * triangleIndices.size();

    auto [stagingBuffer, stagingBufferMemory] 
        = m_Device.CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data;
    Utils::VerifyResult(m_Device->mapMemory(stagingBufferMemory, 0, bufferSize, {}), STEXT("Failed to map index buffer memory!"), &data);
    memcpy(data, triangleIndices.data(), (size_t)bufferSize);
    m_Device->unmapMemory(stagingBufferMemory);

    std::tie(m_IndexBuffer, m_IndexBufferMemory)
        = m_Device.CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                                vk::MemoryPropertyFlagBits::eDeviceLocal);

    Utils::CopyBuffer(this, stagingBuffer, m_IndexBuffer, bufferSize);

    m_Device->destroyBuffer(stagingBuffer);
    m_Device->freeMemory(stagingBufferMemory);
}

void VulkanRHI::CreateUniformBuffer()
{
    vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
    size_t bufferCount = m_Swapchain.GetImageCount();
    m_UniformBuffers.resize(bufferCount);
    m_UniformBuffersMemory.resize(bufferCount);

    for (size_t i = 0; i < bufferCount; i++)
    {
        std::tie(m_UniformBuffers[i], m_UniformBuffersMemory[i]) 
            = m_Device.CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
                                    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    }
}

void VulkanRHI::CreateDescriptorPool()
{
    vk::DescriptorPoolSize poolSize = {
        .descriptorCount = m_Swapchain.GetImageCount(),
    };
    vk::DescriptorPoolCreateInfo poolInfo = {
        .maxSets = m_Swapchain.GetImageCount(),
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize,
    };
    Utils::VerifyResult(m_Device->createDescriptorPool(poolInfo), STEXT("Failed to create descriptor pool!"), &m_DescriptorPool);
}

void VulkanRHI::CreateDescriptorSets()
{
    std::vector<vk::DescriptorSetLayout> layouts(m_Swapchain.GetImageCount(), m_DescriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo = {
        .descriptorPool = m_DescriptorPool,
        .descriptorSetCount = Utils::CastNumType(m_Swapchain.GetImageCount()),
        .pSetLayouts = layouts.data(),
    };
    m_DescriptorSets.resize(m_Swapchain.GetImageCount());
    Utils::VerifyResult(m_Device->allocateDescriptorSets(allocInfo), STEXT("Failed to alloc descriptor set!"), &m_DescriptorSets);
    for (size_t i = 0; i < m_Swapchain.GetImageCount(); i++)
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
    m_ImageAvailableSemaphores.resize(m_MaxFrameCountInFlight);
    m_RenderFinishedSemaphores.resize(m_MaxFrameCountInFlight);
    m_InFlightFences.resize(m_MaxFrameCountInFlight);

    vk::SemaphoreCreateInfo semaphoreInfo = {};
    vk::FenceCreateInfo fenceInfo = {
        .flags = vk::FenceCreateFlagBits::eSignaled,
    };

    for (size_t i = 0; i < m_MaxFrameCountInFlight; i++)
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
                                        .extent = m_Swapchain.GetExtent(),
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
                                SA_LOG_ERROR(STEXT("Failed to acquire swap chain image!"));
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

    Utils::VerifyResult(m_Device.GetQueue(ERHIQueueType::Graphics).submit(submitInfo, m_InFlightFences[m_CurrFrameIndex]), STEXT("Failed to submit draw command buffer!"));

    vk::PresentInfoKHR presentInfo = {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_RenderFinishedSemaphores[m_CurrFrameIndex],
        .swapchainCount = 1,
        .pSwapchains = &m_Swapchain.Native(),
        .pImageIndices = &imageIdx,
    };

    Utils::VerifyResult(m_Device.GetQueue(ERHIQueueType::Present).presentKHR(presentInfo),
                        [this](auto result) {
                            SharedHandle windowSys = g_GlobalContext.windowSys;
                            if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || windowSys->IsFramebufferResized())
                            {
                                windowSys->SetFramebufferResizedToDefault();
                                RecreateSwapchain();
                            } else if (result != vk::Result::eSuccess)
                            {
                                SA_LOG_ERROR(STEXT("Failed to present swap chain image!"));
                            }
                        });

    m_CurrFrameIndex = (m_CurrFrameIndex + 1) % m_MaxFrameCountInFlight;
}


void VulkanRHI::CreateInstance(Out<VulkanInstance> instance, In<RHIConfig> config) noexcept
{
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
    SA_LOG_INFO(STEXT("Vulkan Instance Initialized."));
}

// ==============================================
// Tool Functions
// ==============================================

void VulkanRHI::UpdateUniformBuffer(uint32_t idx)
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float>(currentTime - startTime).count();

    UniformBufferObject ubo = {};
    ubo.modelMatrix = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.viewMatrix = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.projectMatrix = glm::perspective(glm::radians(45.0f), static_cast<float>(m_Swapchain.GetExtent().width) / m_Swapchain.GetExtent().height, 0.1f, 10.0f);
    ubo.projectMatrix[1][1] *= -1;  // for vulkan

    void* data;
    Utils::VerifyResult(m_Device->mapMemory(m_UniformBuffersMemory[idx], 0, sizeof(ubo), {}), STEXT("Failed to map index buffer memory!"), &data);
    memcpy(data, &ubo, sizeof(ubo));
    m_Device->unmapMemory(m_UniformBuffersMemory[idx]);
}

}
