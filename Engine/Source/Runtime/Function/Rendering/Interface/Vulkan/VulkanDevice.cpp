#include "VulkanDevice.h"
#include <set>
namespace Snowy::Ark
{
using Utils = VulkanUtils;

void VulkanDevice::Init(In<VulkanInstance> instance)
{
    m_Owner = &(const_cast<VulkanInstance&>(instance));
    m_Adapter = m_Owner->GetAdapter(0);
    if (!IsDeviceSuitable(m_Adapter))
    {
        for (uint32_t i = 1; i < m_Owner->GetAdapterCount(); i++)
        {
            auto* tempAdapter = m_Owner->GetAdapter(i);
            if (IsDeviceSuitable(tempAdapter))
            {
                m_Adapter = tempAdapter;
                break;
            }
        }
        if (m_Adapter.Get() == m_Owner->GetAdapter(0))
        {
            LOG_ERROR("Failed to find a suitable GPU!");
        }
    }

    auto& indices = m_Adapter->GetQueueFamilyIndices();
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
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(m_RequiredExtensions.size()),
        .ppEnabledExtensionNames = m_RequiredExtensions.data(),
        .pEnabledFeatures = &deviceFeatures,
    };
    if (m_Owner->IsEnableValidationLayers())
    {
        createInfo.setPEnabledLayerNames(m_ValidationLayers);
    } else
    {
        createInfo.setPEnabledLayerNames(nullptr);
    }

    Utils::VerifyResult(m_Adapter->Native().createDevice(createInfo, nullptr), "Failed to create Logical device!", &m_Native);

    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_Native);

    m_Queues[static_cast<int>(ERHIQueueType::Present)] = m_Native.getQueue(indices.present.value(), 0);
    m_Queues[static_cast<int>(ERHIQueueType::Graphics)] = m_Native.getQueue(indices.graphics.value(), 0);

    LOG_INFO("Create Logical Device, Complete.");
}
void VulkanDevice::Destroy()
{

}

void VulkanDevice::PrepareExtensionsAndLayers(In<RHIConfig> config)
{
    m_ValidationLayers = config.vkValidationLayers;
    m_RequiredExtensions = config.vkDeviceExtensions;
}

bool VulkanDevice::CheckDeviceExtensionSupport(ObserverHandle<VulkanAdapter> adapter)
{
    std::set<std::string> requiredExtensions(m_RequiredExtensions.begin(), m_RequiredExtensions.end());
    Utils::VerifyResult(adapter->Native().enumerateDeviceExtensionProperties(nullptr),
                        [&](const auto& result) {
                            auto& [r, extensions] = result;
                            if (r != vk::Result::eSuccess)
                            {
                                LOG_ERROR("Failed to enumerate instance layer properties!");
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

bool VulkanDevice::IsDeviceSuitable(ObserverHandle<VulkanAdapter> adapter)
{
    auto&& indices = adapter->GetQueueFamilyIndices();
    bool extensionsSupported = CheckDeviceExtensionSupport(adapter);
    bool swapchainAdequate = false;
    if (extensionsSupported)
    {
        auto&& swapChainSupport = adapter->GetSwapChainSupportDetails();
        swapchainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    return indices.IsComplete() && extensionsSupported && swapchainAdequate;
}
}