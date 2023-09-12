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

    void Init(ObserverHandle<OwnerType> owner, NativeType native) noexcept;

    auto& Native    () noexcept { return m_Native; }
    auto& Native    () const noexcept { return m_Native; }
    auto& operator* () noexcept { return m_Native; }
    auto& operator* () const noexcept { return m_Native; }
    auto* operator->() noexcept { return &m_Native; }
    auto* operator->() const noexcept { return &m_Native; }
    operator NativeType() const noexcept { return m_Native; }
    operator NativeType::NativeType() const noexcept { return m_Native; }
    ObserverHandle<OwnerType> GetOwner() const noexcept { return m_Owner; }
    ObserverHandle<VulkanRHI> GetContext() const noexcept { return m_Ctx; }

    auto& GetProperties(this auto&& self) noexcept { return self.m_Properties.properties; }
    auto& GetProperties2(this auto&& self) noexcept { return self.m_Properties; }
    auto& GetQueueFamilyIndices(this auto&& self) noexcept { return self.m_QueueFamilyIndices; }

    SwapchainSupportDetails QuerySwapchainSupportDetails() const;

private:
    void QueryProperties() noexcept;
    void QueryQueueFamilyIndices() noexcept;

private:
    NativeType m_Native;
    ObserverHandle<OwnerType> m_Owner;
    ObserverHandle<VulkanRHI> m_Ctx;

    vk::PhysicalDeviceProperties2 m_Properties;
    QueueFamilyIndices m_QueueFamilyIndices;
};
}
