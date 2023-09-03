#pragma once
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanUtils.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanAdapter.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanInstance.h"

namespace Snowy::Ark
{
class VulkanDevice
{
public:
    using NativeType = vk::Device;
    using OwnerType  = VulkanInstance;
public:
    void Init(In<VulkanInstance> instance); 
    void Destroy();
    void PrepareExtensionsAndLayers(In<RHIConfig> config);

    auto& Native    (this auto&& self) noexcept { return self.m_Native; }
    auto& operator* (this auto&& self) noexcept { return self.m_Native; }
    auto* operator->(this auto&& self) noexcept { return &(self.m_Native); }
    operator NativeType() const noexcept { return m_Native; }
    operator NativeType::NativeType() const noexcept { return m_Native; }
    ObserverHandle<OwnerType> GetOwner() const noexcept { return m_Owner; }
    void SetOwner(ObserverHandle<OwnerType> owner) noexcept { m_Owner = owner; }

    ObserverHandle<VulkanAdapter> GetAdapter() const noexcept { return m_Adapter; }
    vk::Queue& GetQueue(ERHIQueueType type) { return m_Queues[static_cast<int>(type)]; }
    const std::vector<const AnsiChar*>& GetValidationLayers() const noexcept { return m_ValidationLayers; }
    const std::vector<const AnsiChar*>& GetRequiredExtensions() const noexcept { return m_RequiredExtensions; }

private:
    bool CheckDeviceExtensionSupport(ObserverHandle<VulkanAdapter> adapter);
    bool IsDeviceSuitable(ObserverHandle<VulkanAdapter> adapter);


private:
    NativeType m_Native;
    ObserverHandle<OwnerType> m_Owner;
    ObserverHandle<VulkanAdapter> m_Adapter;
    std::array<vk::Queue, 2> m_Queues;

    std::vector<const AnsiChar*> m_ValidationLayers;
    std::vector<const AnsiChar*> m_RequiredExtensions;
};
}