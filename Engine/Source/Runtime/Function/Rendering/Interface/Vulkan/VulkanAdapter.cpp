#include "VulkanAdapter.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanInstance.h"
namespace Snowy::Ark
{
using Utils = VulkanUtils;

void VulkanAdapter::QueryProperties()
{
    m_Native.getProperties2(&m_Properties);
}

void VulkanAdapter::QueryQueueFamilyIndices()
{
    std::vector<vk::QueueFamilyProperties> queueFamilies = m_Native.getQueueFamilyProperties();
    int idx = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            m_QueueFamilyIndices.graphics = idx;
        }

        vk::Bool32 presentSupport = m_Native.getSurfaceSupportKHR(idx, m_Owner->GetSurface()).value;
        if (queueFamily.queueCount > 0 && presentSupport)
        {
            m_QueueFamilyIndices.present = idx;
        }

        if (m_QueueFamilyIndices.IsComplete())
        {
            break;
        }
        idx++;
    }
}

void VulkanAdapter::QuerySwapchainSupport()
{
    // 查询基础表面特性
    Utils::VerifyResult(m_Native.getSurfaceCapabilitiesKHR(m_Owner->GetSurface()), "Failed to get Surface Capabilities!", &m_SwapchainSupportDetails.capabilities);
    // 查询表面支持格式
    Utils::VerifyResult(m_Native.getSurfaceFormatsKHR(m_Owner->GetSurface()), "Failed to get Surface Formats!", &m_SwapchainSupportDetails.formats); 
    // 查询支持的呈现方式
    Utils::VerifyResult(m_Native.getSurfacePresentModesKHR(m_Owner->GetSurface()), "Failed to get Surface PresentModes!", &m_SwapchainSupportDetails.presentModes);
}
}