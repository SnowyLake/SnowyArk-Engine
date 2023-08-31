#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/RHI.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanUtils.h"

#include <vulkan/vulkan.hpp>

namespace Snowy::Ark
{
class VulkanInstance;

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphics;
    std::optional<uint32_t> present;

    bool IsComplete()
    {
        return graphics.has_value() && present.has_value();
    }
};
struct SwapChainSupportDetails
{
    vk::SurfaceCapabilitiesKHR capabilities = {};
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

class VulkanAdapter
{    
    friend class VulkanInstance;
protected:
    void QueryProperties();
    void QueryQueueFamilyIndices();
    void QuerySwapChainSupport();

public:
    vk::PhysicalDevice physicalDevice;
    vk::PhysicalDeviceProperties properties;

    QueueFamilyIndices queueFamilyIndices;
    SwapChainSupportDetails swapchainSupportDetails;

    RawHandle<VulkanInstance> instance;
};
}