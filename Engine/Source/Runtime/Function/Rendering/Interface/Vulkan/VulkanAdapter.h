#pragma once
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanUtils.h"

namespace Snowy::Ark
{
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphics;
    std::optional<uint32_t> present;

    bool IsComplete() const noexcept
    {
        return graphics.has_value() && present.has_value();
    }
};
struct SwapchainSupportDetails
{
    vk::SurfaceCapabilitiesKHR capabilities = {};
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

class VulkanInstance;

class VulkanAdapter
{    
    friend class VulkanInstance;
public:
    using NativeType = vk::PhysicalDevice;
    using OwnerType  = VulkanInstance;

    auto& Native    (this auto&& self) noexcept { return self.m_Native; }
    auto& operator* (this auto&& self) noexcept { return self.m_Native; }
    auto* operator->(this auto&& self) noexcept { return &(self.m_Native); }
    operator NativeType() const noexcept { return m_Native; }
    operator NativeType::NativeType() const noexcept { return m_Native; }
    ObserverHandle<OwnerType> GetOwner() const noexcept { return m_Owner; }
    void SetOwner(ObserverHandle<OwnerType> owner) noexcept { m_Owner = owner; }

    vk::PhysicalDeviceProperties2& GetProperties2() noexcept { return m_Properties; }
    vk::PhysicalDeviceProperties& GetProperties() noexcept { return m_Properties.properties; }
    QueueFamilyIndices& GetQueueFamilyIndices() noexcept { return m_QueueFamilyIndices; }
    SwapchainSupportDetails& GetSwapChainSupportDetails() noexcept { return m_SwapchainSupportDetails; }

private:
    void QueryProperties();
    void QueryQueueFamilyIndices();
    void QuerySwapchainSupport();

private:
    NativeType m_Native;
    ObserverHandle<OwnerType> m_Owner;
    
    vk::PhysicalDeviceProperties2 m_Properties;
    QueueFamilyIndices m_QueueFamilyIndices;
    SwapchainSupportDetails m_SwapchainSupportDetails;
};
}