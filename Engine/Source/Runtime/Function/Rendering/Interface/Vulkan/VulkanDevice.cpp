#include "VulkanDevice.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanInstance.h"
#include <set>
namespace Snowy::Ark
{
using Utils = VulkanUtils;

void VulkanDevice::Init(ObserverHandle<OwnerType> owner) noexcept
{
    m_Owner = owner;
    m_Ctx = m_Owner->GetContext();

    m_Adapter = m_Owner->GetAdapter(0);
    if (!IsDeviceSuitable(*m_Adapter))
    {
        for (uint32_t i = 1; i < m_Owner->GetAdapterCount(); i++)
        {
            auto tempAdapter = m_Owner->GetAdapter(i);
            if (IsDeviceSuitable(*tempAdapter))
            {
                m_Adapter = tempAdapter;
                break;
            }
        }
        if (m_Adapter == m_Owner->GetAdapter(0))
        {
            SA_LOG_ERROR(STEXT("Failed to find a suitable GPU!"));
        }
    }

    auto& indices = GetAdapter().GetQueueFamilyIndices();
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { *indices.graphics, *indices.present };
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
        .queueCreateInfoCount = Utils::CastNumType(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = Utils::CastNumType(m_RequiredExtensions.size()),
        .ppEnabledExtensionNames = m_RequiredExtensions.data(),
        .pEnabledFeatures = &deviceFeatures,
    };
    if (m_Owner->EnableValidationLayers())
    {
        createInfo.setPEnabledLayerNames(m_ValidationLayers);
    } else
    {
        createInfo.setPEnabledLayerNames(nullptr);
    }

    Utils::VerifyResult(GetAdapter()->createDevice(createInfo, nullptr), STEXT("Failed to create Logical device!"), &m_Native);

    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_Native);

    m_Queues.resize(2);
    m_Queues[Utils::CastNumType(ERHIQueueType::Present)] = m_Native.getQueue(indices.present.value(), 0);
    m_Queues[Utils::CastNumType(ERHIQueueType::Graphics)] = m_Native.getQueue(indices.graphics.value(), 0);
}

void VulkanDevice::Destroy() noexcept
{
    m_Native.destroy();
}

void VulkanDevice::CreateSwapchain(Out<VulkanSwapchain> swapchain) noexcept
{
    swapchain->Init(this);
    SA_LOG_INFO(STEXT("Vulkan SwapChain, Initialized."));
}

std::tuple<vk::Buffer, vk::DeviceMemory> VulkanDevice::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) noexcept
{
    vk::Buffer buffer;
    vk::DeviceMemory bufferMemory;

    vk::BufferCreateInfo bufferInfo = {
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
    };
    Utils::VerifyResult(m_Native.createBuffer(bufferInfo), STEXT("Failed to create buffer!"), &buffer);

    vk::MemoryRequirements memRequirements;
    m_Native.getBufferMemoryRequirements(buffer, &memRequirements);
    vk::MemoryAllocateInfo allocInfo = {
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties),
    };
    Utils::VerifyResult(m_Native.allocateMemory(allocInfo), STEXT("Failed to allocate vertex buffer memory!"), &bufferMemory);
    Utils::VerifyResult(m_Native.bindBufferMemory(buffer, bufferMemory, 0), STEXT("Failed to bind vertex buffer memory!"));

    return std::make_tuple(buffer, bufferMemory);
}

vk::ShaderModule VulkanDevice::CreateShaderModule(ArrayIn<char> code) noexcept
{
    vk::ShaderModule shaderModule;

    vk::ShaderModuleCreateInfo createInfo = {
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t*>(code.data()),
    };

    Utils::VerifyResult(m_Native.createShaderModule(createInfo), STEXT("Failed to create shader module!"), &shaderModule);

    return shaderModule;
}

uint32_t VulkanDevice::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) noexcept
{
    vk::PhysicalDeviceMemoryProperties props = m_Adapter->Native().getMemoryProperties();
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

bool VulkanDevice::CheckDeviceExtensionSupport(In<VulkanAdapter> adapter) noexcept
{
    std::set<std::string> requiredExtensions(m_RequiredExtensions.begin(), m_RequiredExtensions.end());
    Utils::VerifyResult(adapter->enumerateDeviceExtensionProperties(nullptr),
                        [&](const auto& result) {
                            auto& [r, extensions] = result;
                            if (r != vk::Result::eSuccess)
                            {
                                SA_LOG_ERROR(STEXT("Failed to enumerate instance layer properties!"));
                            } else
                            {
                                for (const auto& extension : extensions)
                                {
                                    requiredExtensions.erase(extension.extensionName);
                                }
                            }
                        });
    return requiredExtensions.empty();
}

bool VulkanDevice::IsDeviceSuitable(In<VulkanAdapter> adapter) noexcept
{
    auto&& indices = adapter.GetQueueFamilyIndices();
    bool extensionsSupported = CheckDeviceExtensionSupport(adapter);
    bool swapchainAdequate = false;
    if (extensionsSupported)
    {
        auto&& swapChainSupport = adapter.QuerySwapchainSupportDetails();
        swapchainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    return indices.IsComplete() && extensionsSupported && swapchainAdequate;
}
}
