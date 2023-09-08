#pragma once
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanUtils.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanAdapter.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanSwapchain.h"

namespace Snowy::Ark
{
class VulkanInstance;
class VulkanDevice
{
public:
    using NativeType = vk::Device;
    using OwnerType  = VulkanInstance;
public:
    void Init(ObserverHandle<OwnerType> owner) noexcept; 
    void Destroy() noexcept;
    void PrepareExtensionsAndLayers(In<RHIConfig> config) noexcept;

    auto& Native    (this auto&& self) noexcept { return self.m_Native; }
    auto& operator* (this auto&& self) noexcept { return self.m_Native; }
    auto* operator->(this auto&& self) noexcept { return &(self.m_Native); }
    operator NativeType() const noexcept { return m_Native; }
    operator NativeType::NativeType() const noexcept { return m_Native; }
    ObserverHandle<OwnerType> GetOwner() const noexcept { return m_Owner; }
    ObserverHandle<VulkanRHI> GetContext() const noexcept { return m_Ctx; }

    VulkanAdapter& GetAdapter() const noexcept { return *m_Adapter; }
    vk::Queue& GetQueue(ERHIQueueType type) { return m_Queues[static_cast<size_t>(type)]; }
    const std::vector<const AnsiChar*>& GetValidationLayers() const noexcept { return m_ValidationLayers; }
    const std::vector<const AnsiChar*>& GetRequiredExtensions() const noexcept { return m_RequiredExtensions; }

    VulkanSwapchain CreateSwapchain() noexcept;

private:
    bool CheckDeviceExtensionSupport(In<VulkanAdapter> adapter) noexcept;
    bool IsDeviceSuitable(In<VulkanAdapter> adapter) noexcept;

private:
    NativeType m_Native;
    ObserverHandle<OwnerType> m_Owner;
    ObserverHandle<VulkanRHI> m_Ctx;

    ObserverHandle<VulkanAdapter> m_Adapter;
    std::vector<vk::Queue> m_Queues;

    std::vector<const AnsiChar*> m_ValidationLayers;
    std::vector<const AnsiChar*> m_RequiredExtensions;
};
}
