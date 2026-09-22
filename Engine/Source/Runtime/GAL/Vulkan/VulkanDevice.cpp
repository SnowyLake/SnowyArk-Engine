#include <Runtime/GAL/Vulkan/VulkanDevice.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include <Runtime/Core/CleanupWait.h>
#include <Runtime/Core/Log.h>
#include <Runtime/GAL/Buffer.h>
#include <Runtime/GAL/FrameBegin.h>
#include <Runtime/GAL/PipelineState.h>
#include <Runtime/GAL/Vulkan/VulkanBuffer.h>
#include <Runtime/GAL/Vulkan/VulkanPipelineState.h>
#include <Runtime/GAL/Vulkan/VulkanSwapChain.h>
#include <Runtime/GAL/Vulkan/VulkanUtils.h>
#include <Runtime/Platform/Window.h>

namespace SnowyArk
{

std::unique_ptr<GraphicsDevice> VulkanDevice::Create()
{
    return std::make_unique<VulkanDevice>();
}

VulkanDevice::~VulkanDevice() noexcept
{
    Shutdown();
}

bool VulkanDevice::Initialize(const WindowNativeHandle& nativeHandle, const std::span<const char* const> instanceExtensions)
{
    CreateInstance(instanceExtensions);
    CreateSurface(nativeHandle);
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateCommandPool();
    CreateFrameResources();
    Log::Info("Vulkan device initialized.");
    return true;
}

std::unique_ptr<SwapChain> VulkanDevice::CreateSwapChain(const Window& window)
{
    auto swapChain = std::make_unique<VulkanSwapChain>(m_PhysicalDevice, m_Device, m_Surface, m_GraphicsQueueFamily, m_PresentQueueFamily, window.GetWidth(), window.GetHeight());
    EnsureRenderFinishedSemaphores(swapChain->GetImageCount());
    return swapChain;
}

std::unique_ptr<PipelineState> VulkanDevice::CreateGraphicsPipeline(const GraphicsPipelineDesc& desc)
{
    if (desc.shaderSpirv.empty() || desc.vertexEntryName == nullptr || desc.fragmentEntryName == nullptr || desc.vertexEntryName[0] == '\0' || desc.fragmentEntryName[0] == '\0')
    {
        Log::Fatal("Graphics pipeline requires shader code and non-empty entry names.");
    }
    const auto limits = m_PhysicalDevice.getProperties().limits;
    if (desc.vertexBindings.size() > limits.maxVertexInputBindings || desc.vertexAttributes.size() > limits.maxVertexInputAttributes)
    {
        Log::Fatal("Vertex layout exceeds device binding or attribute limits.");
    }
    const vk::ShaderModuleCreateInfo shaderModuleInfo {
        .codeSize = desc.shaderSpirv.size_bytes(),
        .pCode = desc.shaderSpirv.data(),
    };
    vk::raii::ShaderModule shaderModule(m_Device, shaderModuleInfo);

    const vk::PipelineShaderStageCreateInfo stages[] {
        { .stage = vk::ShaderStageFlagBits::eVertex, .module = shaderModule, .pName = desc.vertexEntryName },
        { .stage = vk::ShaderStageFlagBits::eFragment, .module = shaderModule, .pName = desc.fragmentEntryName },
    };

    std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
    bindingDescriptions.reserve(desc.vertexBindings.size());
    for (const VertexBinding& binding : desc.vertexBindings)
    {
        if (binding.binding >= limits.maxVertexInputBindings || binding.stride > limits.maxVertexInputBindingStride ||
            (binding.inputRate != VertexInputRate::PerVertex && binding.inputRate != VertexInputRate::PerInstance) ||
            std::ranges::any_of(bindingDescriptions, [&](const auto& previous) { return previous.binding == binding.binding; }))
        {
            Log::Fatal("Invalid or duplicate vertex binding description.");
        }
        const vk::VertexInputRate inputRate = binding.inputRate == VertexInputRate::PerInstance ? vk::VertexInputRate::eInstance : vk::VertexInputRate::eVertex;
        bindingDescriptions.push_back({ .binding = binding.binding, .stride = binding.stride, .inputRate = inputRate });
    }

    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
    attributeDescriptions.reserve(desc.vertexAttributes.size());
    for (const VertexAttribute& attribute : desc.vertexAttributes)
    {
        if (attribute.location >= limits.maxVertexInputAttributes || attribute.offset > limits.maxVertexInputAttributeOffset ||
            std::ranges::none_of(desc.vertexBindings, [&](const auto& binding) { return binding.binding == attribute.binding; }) ||
            std::ranges::any_of(attributeDescriptions, [&](const auto& previous) { return previous.location == attribute.location; }))
        {
            Log::Fatal("Invalid vertex attribute location, offset, or binding.");
        }
        const auto format = VulkanUtils::ToVkFormat(attribute.format);
        if (!(m_PhysicalDevice.getFormatProperties(format).bufferFeatures & vk::FormatFeatureFlagBits::eVertexBuffer))
        {
            Log::Fatal("Vertex attribute format is not supported for vertex input.");
        }
        attributeDescriptions.push_back({
            .location = attribute.location,
            .binding = attribute.binding,
            .format = format,
            .offset = attribute.offset,
        });
    }

    const vk::PipelineVertexInputStateCreateInfo vertexInput {
        .vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size()),
        .pVertexBindingDescriptions = bindingDescriptions.data(),
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data(),
    };
    constexpr vk::PipelineInputAssemblyStateCreateInfo inputAssembly { .topology = vk::PrimitiveTopology::eTriangleList };
    constexpr vk::PipelineViewportStateCreateInfo viewportState { .viewportCount = 1, .scissorCount = 1 };
    constexpr vk::PipelineRasterizationStateCreateInfo rasterizer {
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = vk::False,
        .lineWidth = 1.0f,
    };
    constexpr vk::PipelineMultisampleStateCreateInfo multisampling { .rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False };
    constexpr vk::PipelineColorBlendAttachmentState colorBlendAttachment {
        .blendEnable = vk::False,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
    };
    const vk::PipelineColorBlendStateCreateInfo colorBlending {
        .logicOpEnable = vk::False,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
    };
    constexpr vk::DynamicState dynamicStates[] { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    const vk::PipelineDynamicStateCreateInfo dynamicState {
        .dynamicStateCount = 2,
        .pDynamicStates = dynamicStates,
    };
    constexpr vk::PipelineLayoutCreateInfo pipelineLayoutInfo {};
    vk::raii::PipelineLayout pipelineLayout(m_Device, pipelineLayoutInfo);

    const vk::Format colorFormat = VulkanUtils::ToVkFormat(desc.colorFormat);
    vk::PipelineRenderingCreateInfo renderingInfo {
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &colorFormat,
    };
    const vk::GraphicsPipelineCreateInfo pipelineInfo {
        .pNext = &renderingInfo,
        .stageCount = 2,
        .pStages = stages,
        .pVertexInputState = &vertexInput,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = pipelineLayout,
        .renderPass = nullptr,
    };

    vk::raii::Pipeline pipeline(m_Device, nullptr, pipelineInfo);
    return std::make_unique<VulkanPipelineState>(std::move(pipelineLayout), std::move(pipeline));
}

std::unique_ptr<Buffer> VulkanDevice::CreateBuffer(const BufferDesc& desc)
{
    if (desc.initialData.empty())
    {
        Log::Error("Buffer initial data is empty.");
        return nullptr;
    }

    vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eTransferDst;
    vk::PipelineStageFlags2 dstStage = vk::PipelineStageFlagBits2::eVertexAttributeInput;
    vk::AccessFlags2 dstAccess = vk::AccessFlagBits2::eVertexAttributeRead;
    switch (desc.usage)
    {
    case BufferUsage::Vertex:
        usage |= vk::BufferUsageFlagBits::eVertexBuffer;
        dstStage = vk::PipelineStageFlagBits2::eVertexAttributeInput;
        dstAccess = vk::AccessFlagBits2::eVertexAttributeRead;
        break;
    case BufferUsage::Index:
        usage |= vk::BufferUsageFlagBits::eIndexBuffer;
        dstStage = vk::PipelineStageFlagBits2::eIndexInput;
        dstAccess = vk::AccessFlagBits2::eIndexRead;
        break;
    default:
        Log::Fatal("Unsupported buffer usage.");
    }

    const vk::DeviceSize size = desc.initialData.size();
    AllocatedBuffer staging = CreateAllocatedBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    void* const mapped = staging.memory.mapMemory(0, size);
    std::memcpy(mapped, desc.initialData.data(), static_cast<std::size_t>(size));
    staging.memory.unmapMemory();

    AllocatedBuffer deviceBuffer = CreateAllocatedBuffer(size, usage, vk::MemoryPropertyFlagBits::eDeviceLocal);
    CopyBuffer(*staging.buffer, *deviceBuffer.buffer, size, dstStage, dstAccess);
    return std::make_unique<VulkanBuffer>(std::move(deviceBuffer.memory), std::move(deviceBuffer.buffer), size, desc.usage);
}

CommandBuffer* VulkanDevice::BeginFrame(SwapChain& swapChain)
{
    auto& vulkanSwapChain = static_cast<VulkanSwapChain&>(swapChain);
    if (!vulkanSwapChain.RecreateIfNeeded())
    {
        return nullptr;
    }

    EnsureRenderFinishedSemaphores(vulkanSwapChain.GetImageCount());

    const auto waitResult = m_Device.waitForFences(*m_InFlightFences[m_FrameIndex], vk::True, UINT64_MAX);
    if (waitResult != vk::Result::eSuccess)
    {
        Log::Fatal("Failed to wait for the in-flight fence.");
    }

    const SwapChainAcquireResult acquired = vulkanSwapChain.AcquireNextImage(*m_ImageAvailableSemaphores[m_FrameIndex]);
    const AcquiredFrameAction action = AcquiredFrameAction::Decide(acquired.status, acquired.imageIndex);
    if (action.requestRecreation)
    {
        vulkanSwapChain.RequestRecreation();
    }
    if (!action.recordAndPresent)
    {
        vulkanSwapChain.RecreateIfNeeded();
        return nullptr;
    }

    m_ImageIndex = action.imageIndex;
    m_Device.resetFences(*m_InFlightFences[m_FrameIndex]);

    const vk::CommandBuffer commandBuffer = m_VkCommandBuffers[m_FrameIndex];
    commandBuffer.reset();
    commandBuffer.begin(vk::CommandBufferBeginInfo {});
    m_CommandBuffers[m_FrameIndex]->BindTarget(vulkanSwapChain.GetImage(m_ImageIndex), vulkanSwapChain.GetImageView(m_ImageIndex), vulkanSwapChain.GetVkExtent());
    return m_CommandBuffers[m_FrameIndex].get();
}

void VulkanDevice::EndFrame(SwapChain& swapChain)
{
    auto& vulkanSwapChain = static_cast<VulkanSwapChain&>(swapChain);
    const vk::CommandBuffer commandBuffer = m_CommandBuffers[m_FrameIndex]->GetHandle();
    commandBuffer.end();

    constexpr vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    const vk::SubmitInfo submitInfo {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*m_ImageAvailableSemaphores[m_FrameIndex],
        .pWaitDstStageMask = &waitStage,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*m_RenderFinishedSemaphores[m_ImageIndex],
    };
    m_GraphicsQueue.submit(submitInfo, *m_InFlightFences[m_FrameIndex]);

    const vk::SwapchainKHR swapchainHandle = vulkanSwapChain.GetHandle();
    const vk::PresentInfoKHR presentInfo {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*m_RenderFinishedSemaphores[m_ImageIndex],
        .swapchainCount = 1,
        .pSwapchains = &swapchainHandle,
        .pImageIndices = &m_ImageIndex,
    };

    try
    {
        if (const vk::Result presentResult = m_PresentQueue.presentKHR(presentInfo); presentResult == vk::Result::eSuboptimalKHR)
        {
            vulkanSwapChain.RequestRecreation();
        }
    }
    catch (const vk::OutOfDateKHRError&)
    {
        vulkanSwapChain.RequestRecreation();
    }

    m_FrameIndex = (m_FrameIndex + 1) % k_MaxFramesInFlight;
}

void VulkanDevice::WaitIdle()
{
    if (m_Device != nullptr)
    {
        m_Device.waitIdle();
    }
}

void VulkanDevice::Shutdown() noexcept
{
    CleanupWait::TryWait([this] { WaitIdle(); });
    m_CommandBuffers.clear();
    m_InFlightFences.clear();
    m_RenderFinishedSemaphores.clear();
    m_ImageAvailableSemaphores.clear();
    m_VkCommandBuffers.clear();
    m_CommandPool = nullptr;
    m_UploadCommandPool = nullptr;
    m_PresentQueue = nullptr;
    m_GraphicsQueue = nullptr;
    m_Device = nullptr;
    m_PhysicalDevice = nullptr;
    m_Surface = nullptr;
    m_DebugMessenger = nullptr;
    m_Instance = nullptr;
}

void VulkanDevice::CreateInstance(const std::span<const char* const> instanceExtensions)
{
    if (m_Context.enumerateInstanceVersion() < VK_API_VERSION_1_4)
    {
        Log::Fatal("The Vulkan loader does not support Vulkan 1.4. Install LunarG Vulkan SDK 1.4 or later.");
    }

    std::vector<const char*> extensions(instanceExtensions.begin(), instanceExtensions.end());
    std::vector<const char*> layers;
    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo = MakeDebugMessengerCreateInfo();

    if (k_EnableValidation)
    {
        const std::vector<vk::LayerProperties> layerProperties = m_Context.enumerateInstanceLayerProperties();
        if (!HasLayer(layerProperties, "VK_LAYER_KHRONOS_validation"))
        {
            Log::Fatal("VK_LAYER_KHRONOS_validation is not available. Install the LunarG Vulkan SDK.");
        }
        layers.push_back("VK_LAYER_KHRONOS_validation");
        if (!HasExtension(m_Context.enumerateInstanceExtensionProperties(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
        {
            Log::Fatal("VK_EXT_debug_utils is not available.");
        }
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    constexpr vk::ApplicationInfo appInfo {
        .pApplicationName = "SnowyArk",
        .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
        .pEngineName = "SnowyArk",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = vk::ApiVersion14,
    };

    constexpr vk::ValidationFeatureEnableEXT enabledValidationFeatures[] { vk::ValidationFeatureEnableEXT::eSynchronizationValidation };
    vk::ValidationFeaturesEXT validationFeatures {
        .pNext = &debugCreateInfo,
        .enabledValidationFeatureCount = 1,
        .pEnabledValidationFeatures = enabledValidationFeatures,
    };
    const vk::InstanceCreateInfo createInfo {
        .pNext = k_EnableValidation ? &validationFeatures : nullptr,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.empty() ? nullptr : layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    m_Instance = vk::raii::Instance(m_Context, createInfo);
    if (k_EnableValidation)
    {
        m_DebugMessenger = vk::raii::DebugUtilsMessengerEXT(m_Instance, debugCreateInfo);
    }
    Log::Info("Created Vulkan instance.");
}

void VulkanDevice::CreateSurface(const WindowNativeHandle& nativeHandle)
{
    const vk::Win32SurfaceCreateInfoKHR surfaceInfo {
        .hinstance = static_cast<HINSTANCE>(nativeHandle.hinstance),
        .hwnd = static_cast<HWND>(nativeHandle.hwnd),
    };
    m_Surface = vk::raii::SurfaceKHR(m_Instance, surfaceInfo);
}

void VulkanDevice::PickPhysicalDevice()
{
    const std::vector<vk::raii::PhysicalDevice> devices = m_Instance.enumeratePhysicalDevices();
    for (const vk::raii::PhysicalDevice& device : devices)
    {
        if (device.getProperties().apiVersion < VK_API_VERSION_1_4)
        {
            continue;
        }

        const auto featureChain = device.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features>();
        const auto& vulkan13Features = featureChain.get<vk::PhysicalDeviceVulkan13Features>();
        if (!vulkan13Features.dynamicRendering || !vulkan13Features.synchronization2)
        {
            continue;
        }

        const std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
        uint32_t graphicsFamily = UINT32_MAX;
        uint32_t presentFamily = UINT32_MAX;
        for (uint32_t index = 0; index < queueFamilies.size(); ++index)
        {
            if (queueFamilies[index].queueFlags & vk::QueueFlagBits::eGraphics)
            {
                graphicsFamily = index;
            }
            if (device.getSurfaceSupportKHR(index, *m_Surface))
            {
                presentFamily = index;
            }
            if (graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX)
            {
                break;
            }
        }
        if (graphicsFamily == UINT32_MAX || presentFamily == UINT32_MAX)
        {
            continue;
        }

        if (!HasExtension(device.enumerateDeviceExtensionProperties(), VK_KHR_SWAPCHAIN_EXTENSION_NAME))
        {
            continue;
        }
        if (device.getSurfacePresentModesKHR(*m_Surface).empty() || !VulkanUtils::HasRequiredSwapSurfaceFormat(device.getSurfaceFormatsKHR(*m_Surface)))
        {
            continue;
        }

        m_PhysicalDevice = device;
        m_GraphicsQueueFamily = graphicsFamily;
        m_PresentQueueFamily = presentFamily;
        const auto properties = device.getProperties();
        Log::Info("Selected GPU '{}' (Vulkan {}.{}.{}).", static_cast<const char*>(properties.deviceName), VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion),
                  VK_VERSION_PATCH(properties.apiVersion));
        return;
    }

    Log::Fatal("No suitable Vulkan 1.4 GPU was found. A candidate must report dynamicRendering and synchronization2, and the surface must support R8G8B8A8Srgb with SrgbNonlinear.");
}

void VulkanDevice::CreateLogicalDevice()
{
    constexpr float queuePriority = 1.0f;
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    const std::set<uint32_t> uniqueFamilies = { m_GraphicsQueueFamily, m_PresentQueueFamily };
    for (const uint32_t family : uniqueFamilies)
    {
        queueCreateInfos.push_back(vk::DeviceQueueCreateInfo { .queueFamilyIndex = family, .queueCount = 1, .pQueuePriorities = &queuePriority });
    }

    vk::PhysicalDeviceVulkan13Features vulkan13Features { .synchronization2 = vk::True, .dynamicRendering = vk::True };
    constexpr const char* swapchainExtension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    const vk::DeviceCreateInfo deviceInfo {
        .pNext = &vulkan13Features,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = &swapchainExtension,
    };

    m_Device = vk::raii::Device(m_PhysicalDevice, deviceInfo);
    m_GraphicsQueue = vk::raii::Queue(m_Device, m_GraphicsQueueFamily, 0);
    m_PresentQueue = vk::raii::Queue(m_Device, m_PresentQueueFamily, 0);
}

void VulkanDevice::CreateCommandPool()
{
    const vk::CommandPoolCreateInfo poolInfo {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = m_GraphicsQueueFamily,
    };
    m_CommandPool = vk::raii::CommandPool(m_Device, poolInfo);

    const vk::CommandPoolCreateInfo uploadPoolInfo {
        .flags = vk::CommandPoolCreateFlagBits::eTransient,
        .queueFamilyIndex = m_GraphicsQueueFamily,
    };
    m_UploadCommandPool = vk::raii::CommandPool(m_Device, uploadPoolInfo);
}

VulkanDevice::AllocatedBuffer VulkanDevice::CreateAllocatedBuffer(const vk::DeviceSize size, const vk::BufferUsageFlags usage, const vk::MemoryPropertyFlags properties)
{
    const vk::BufferCreateInfo bufferInfo {
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
    };
    vk::raii::Buffer buffer(m_Device, bufferInfo);
    const vk::MemoryRequirements requirements = buffer.getMemoryRequirements();
    const vk::MemoryAllocateInfo allocateInfo {
        .allocationSize = requirements.size,
        .memoryTypeIndex = FindMemoryType(requirements.memoryTypeBits, properties),
    };
    // ponytail: one VkDeviceMemory per buffer. Ceiling is maxMemoryAllocationCount (can be 4096). Upgrade path is suballocation.
    vk::raii::DeviceMemory memory(m_Device, allocateInfo);
    buffer.bindMemory(*memory, 0);
    return { .memory = std::move(memory), .buffer = std::move(buffer) };
}

uint32_t VulkanDevice::FindMemoryType(const uint32_t typeFilter, const vk::MemoryPropertyFlags properties) const
{
    const vk::PhysicalDeviceMemoryProperties memoryProperties = m_PhysicalDevice.getMemoryProperties();
    for (uint32_t index = 0; index < memoryProperties.memoryTypeCount; ++index)
    {
        const bool supported = (typeFilter & (1u << index)) != 0;
        const bool matches = (memoryProperties.memoryTypes[index].propertyFlags & properties) == properties;
        if (supported && matches)
        {
            return index;
        }
    }
    Log::Fatal("Failed to find a suitable Vulkan memory type.");
}

void VulkanDevice::CopyBuffer(const vk::Buffer source, const vk::Buffer destination, const vk::DeviceSize size, const vk::PipelineStageFlags2 dstStage, const vk::AccessFlags2 dstAccess)
{
    const vk::CommandBufferAllocateInfo allocateInfo {
        .commandPool = m_UploadCommandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };
    std::vector<vk::raii::CommandBuffer> commandBuffers = m_Device.allocateCommandBuffers(allocateInfo);
    vk::raii::CommandBuffer& commandBuffer = commandBuffers.front();
    commandBuffer.begin(vk::CommandBufferBeginInfo { .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
    const vk::BufferCopy region { .srcOffset = 0, .dstOffset = 0, .size = size };
    commandBuffer.copyBuffer(source, destination, region);

    const vk::BufferMemoryBarrier2 barrier {
        .srcStageMask = vk::PipelineStageFlagBits2::eCopy,
        .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
        .dstStageMask = dstStage,
        .dstAccessMask = dstAccess,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .buffer = destination,
        .offset = 0,
        .size = size,
    };
    const vk::DependencyInfo dependency { .bufferMemoryBarrierCount = 1, .pBufferMemoryBarriers = &barrier };
    commandBuffer.pipelineBarrier2(dependency);
    commandBuffer.end();

    const vk::CommandBuffer commandHandle = *commandBuffer;
    const vk::SubmitInfo submitInfo { .commandBufferCount = 1, .pCommandBuffers = &commandHandle };
    vk::raii::Fence fence(m_Device, vk::FenceCreateInfo {});
    m_GraphicsQueue.submit(submitInfo, *fence);
    try
    {
        const vk::Result waitResult = m_Device.waitForFences(*fence, vk::True, UINT64_MAX);
        if (waitResult != vk::Result::eSuccess)
        {
            Log::Fatal("Failed to wait for a buffer upload.");
        }
    }
    catch (...)
    {
        // Keep the fence, command buffer, and both buffers alive through the fault-path wait.
        CleanupWait::TryWait([this] { WaitIdle(); });
        throw;
    }
}

void VulkanDevice::CreateFrameResources()
{
    const vk::CommandBufferAllocateInfo allocInfo {
        .commandPool = m_CommandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = k_MaxFramesInFlight,
    };
    m_VkCommandBuffers = vk::raii::CommandBuffers(m_Device, allocInfo);

    m_CommandBuffers.clear();
    m_ImageAvailableSemaphores.clear();
    m_InFlightFences.clear();
    m_CommandBuffers.reserve(k_MaxFramesInFlight);
    for (uint32_t index = 0; index < k_MaxFramesInFlight; ++index)
    {
        m_CommandBuffers.push_back(std::make_unique<VulkanCommandBuffer>(m_VkCommandBuffers[index], *m_Device, m_PhysicalDevice.getProperties().limits.maxVertexInputBindings));
        m_ImageAvailableSemaphores.emplace_back(m_Device, vk::SemaphoreCreateInfo {});
        m_InFlightFences.emplace_back(m_Device, vk::FenceCreateInfo { .flags = vk::FenceCreateFlagBits::eSignaled });
    }
}

void VulkanDevice::EnsureRenderFinishedSemaphores(const uint32_t imageCount)
{
    if (m_RenderFinishedSemaphores.size() == imageCount)
    {
        return;
    }

    std::vector<vk::raii::Semaphore> semaphores;
    semaphores.reserve(imageCount);
    for (uint32_t index = 0; index < imageCount; ++index)
    {
        semaphores.emplace_back(m_Device, vk::SemaphoreCreateInfo {});
    }

    WaitIdle();
    m_RenderFinishedSemaphores = std::move(semaphores);
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL VulkanDevice::DebugCallback(const vk::DebugUtilsMessageSeverityFlagBitsEXT severity, const vk::DebugUtilsMessageTypeFlagsEXT,
                                                             const vk::DebugUtilsMessengerCallbackDataEXT* const callbackData, void* const)
{
    const char* const message = (callbackData != nullptr && callbackData->pMessage != nullptr) ? callbackData->pMessage : "";
    try
    {
        if (severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
        {
            Log::Error("Vulkan: {}", message);
        }
        else if (severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
        {
            Log::Warning("Vulkan: {}", message);
        }
        else
        {
            Log::Info("Vulkan: {}", message);
        }
    }
    catch (...)
    {
        try
        {
            std::fputs("Vulkan: ", stderr);
            std::fputs(message, stderr);
            std::fputc('\n', stderr);
        }
        catch (...)
        {
        }
    }
    return vk::False;
}

bool VulkanDevice::HasLayer(const std::vector<vk::LayerProperties>& layers, const char* const name)
{
    return std::ranges::any_of(layers, [name](const vk::LayerProperties& layer) { return std::strcmp(layer.layerName, name) == 0; });
}

bool VulkanDevice::HasExtension(const std::vector<vk::ExtensionProperties>& extensions, const char* const name)
{
    return std::ranges::any_of(extensions, [name](const vk::ExtensionProperties& extension) { return std::strcmp(extension.extensionName, name) == 0; });
}

vk::DebugUtilsMessengerCreateInfoEXT VulkanDevice::MakeDebugMessengerCreateInfo()
{
    return vk::DebugUtilsMessengerCreateInfoEXT {
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        .pfnUserCallback = &DebugCallback,
    };
}

}
