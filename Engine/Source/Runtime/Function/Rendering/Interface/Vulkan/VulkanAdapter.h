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
    friend struct IVulkanComponentWapper;
public:
    using NativeType = vk::PhysicalDevice;
    using OwnerType  = VulkanInstance;

    auto& GetNative (this auto&& self) noexcept { return self.m_Native; }
    auto& operator* (this auto&& self) noexcept { return self.m_Native; }
    auto* operator->(this auto&& self) noexcept { return &(self.m_Native); }
    operator NativeType() const noexcept { return m_Native; }
    operator NativeType::NativeType() const noexcept { return m_Native; }
    void SetOwner(ObserverHandle<OwnerType> owner) { m_Owner = owner; }
    ObserverHandle<OwnerType> GetOwner() const noexcept { return m_Owner; }

    const vk::PhysicalDeviceProperties& GetProperties() const noexcept { return m_Properties.properties; }

private:
    void QueryProperties();
    void QueryQueueFamilyIndices();
    void QuerySwapChainSupport();

private:
    NativeType m_Native;
    ObserverHandle<OwnerType> m_Owner;
    
    vk::PhysicalDeviceProperties2 m_Properties;
    QueueFamilyIndices m_QueueFamilyIndices;
    SwapChainSupportDetails m_SwapchainSupportDetails;
};
}