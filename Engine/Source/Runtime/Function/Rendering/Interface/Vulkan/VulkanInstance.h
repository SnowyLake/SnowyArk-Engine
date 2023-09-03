#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/RHI.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanUtils.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanAdapter.h"

#include <vulkan/vulkan.hpp>

namespace Snowy::Ark
{
class VulkanInstance
{
    friend struct IVulkanComponentWapper; 
public:
    using NativeType = vk::Instance;
    using OwnerType  = VulkanRHI;

    void Init(ObserverHandle<VulkanRHI> vkContext, vk::InstanceCreateInfo createInfo, vk::Optional<const vk::AllocationCallbacks> allocator = nullptr);
    void Destroy();
    void PrepareExtensionsAndLayers(In<RHIConfig> config);

    auto& GetNative (this auto&& self) noexcept { return self.m_Native; }
    auto& operator* (this auto&& self) noexcept { return self.m_Native; }
    auto* operator->(this auto&& self) noexcept { return &(self.m_Native); }
    operator NativeType() const noexcept { return m_Native; }
    operator NativeType::NativeType() const noexcept { return m_Native; }
    void SetOwner(ObserverHandle<OwnerType> owner) { m_Owner = owner; }
    ObserverHandle<OwnerType> GetOwner() const noexcept { return m_Owner; }

    bool IsEnableValidationLayers() const noexcept { return m_EnableValidationLayers; }
    const std::vector<const AnsiChar*>& GetValidationLayers() const noexcept { return m_ValidationLayers; }
    const std::vector<const AnsiChar*>& GetRequiredExtensions() const noexcept { return m_RequiredExtensions; }
    const vk::SurfaceKHR& GetVkSurface() const noexcept { return m_Surface; }
    const VulkanAdapter& GetAdapter(uint32_t idx) const noexcept { return m_Adapters[idx]; }
    uint32_t GetAdapterCount() const noexcept { return static_cast<uint32_t>(m_Adapters.size()); }

private:
    void FetchAllAdapters();
    void SetupDebugCallback();
    void CreateSurface();

    bool CheckValidationLayersSupport(ArrayIn<const AnsiChar*> validationLayers);

public:

private:
    NativeType m_Native;
    ObserverHandle<OwnerType> m_Owner;

    bool m_EnableValidationLayers;
    std::vector<const AnsiChar*> m_ValidationLayers;
    std::vector<const AnsiChar*> m_RequiredExtensions;

    vk::SurfaceKHR m_Surface;
    vk::DebugUtilsMessengerEXT m_Callback;
    std::vector<VulkanAdapter> m_Adapters;
};
}