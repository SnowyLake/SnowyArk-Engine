#pragma once
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanUtils.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanAdapter.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanDevice.h"

namespace Snowy::Ark
{
class VulkanInstance
{
public:
    using NativeType = vk::Instance;
    using OwnerType  = VulkanRHI;

    void Init(ObserverHandle<VulkanRHI> vkContext, vk::InstanceCreateInfo createInfo, vk::Optional<const vk::AllocationCallbacks> allocator = nullptr);
    void Destroy();
    void PrepareExtensionsAndLayers(In<RHIConfig> config);

    auto& Native    (this auto&& self) noexcept { return self.m_Native; }
    auto& operator* (this auto&& self) noexcept { return self.m_Native; }
    auto* operator->(this auto&& self) noexcept { return &(self.m_Native); }
    operator NativeType() const noexcept { return m_Native; }
    operator NativeType::NativeType() const noexcept { return m_Native; }
    ObserverHandle<OwnerType> GetOwner() const noexcept { return m_Owner; }
    void SetOwner(ObserverHandle<OwnerType> owner) noexcept { m_Owner = owner; }

    bool IsEnableValidationLayers() const noexcept { return m_EnableValidationLayers; }
    const std::vector<const AnsiChar*>& GetValidationLayers() const noexcept { return m_ValidationLayers; }
    const std::vector<const AnsiChar*>& GetRequiredExtensions() const noexcept { return m_RequiredExtensions; }
    const vk::SurfaceKHR& GetSurface() const noexcept { return m_Surface; }
    VulkanAdapter* GetAdapter(uint32_t idx) noexcept { return &(m_Adapters[idx]); }
    uint32_t GetAdapterCount() const noexcept { return static_cast<uint32_t>(m_Adapters.size()); }


private:
    void CollectAdapters();
    void SetupDebugCallback();
    void CreateSurface();

    bool CheckValidationLayersSupport(ArrayIn<const AnsiChar*> validationLayers);

private:
    NativeType m_Native;
    ObserverHandle<OwnerType> m_Owner;

    vk::SurfaceKHR m_Surface;
    vk::DebugUtilsMessengerEXT m_Callback;
    std::vector<VulkanAdapter> m_Adapters;

    bool m_EnableValidationLayers;
    std::vector<const AnsiChar*> m_ValidationLayers;
    std::vector<const AnsiChar*> m_RequiredExtensions;
};
}