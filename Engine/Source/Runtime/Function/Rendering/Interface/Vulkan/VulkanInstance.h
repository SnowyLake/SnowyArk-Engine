﻿#pragma once
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

    void Init(ObserverHandle<OwnerType> owner, vk::InstanceCreateInfo createInfo) noexcept;
    void Destroy() noexcept;
    void PrepareExtensionsAndLayers(In<RHIConfig> config) noexcept;

    auto& Native    (this auto&& self) noexcept { return self.m_Native; }
    auto& operator* (this auto&& self) noexcept { return self.m_Native; }
    auto* operator->(this auto&& self) noexcept { return &(self.m_Native); }
    operator NativeType() const noexcept { return m_Native; }
    operator NativeType::NativeType() const noexcept { return m_Native; }
    ObserverHandle<OwnerType> GetOwner() const noexcept { return m_Owner; }
    ObserverHandle<VulkanRHI> GetContext() const noexcept { return m_Ctx; }

    bool IsEnableValidationLayers() const noexcept { return m_EnableValidationLayers; }
    const std::vector<const AnsiChar*>& GetValidationLayers() const noexcept { return m_ValidationLayers; }
    const std::vector<const AnsiChar*>& GetRequiredExtensions() const noexcept { return m_RequiredExtensions; }
    const vk::SurfaceKHR& GetSurface() const noexcept { return m_Surface; }
    ObserverHandle<VulkanAdapter> GetAdapter(uint32_t idx) noexcept { return &(m_Adapters[idx]); }
    uint32_t GetAdapterCount() const noexcept { return static_cast<uint32_t>(m_Adapters.size()); }

    void CreateDevice(Out<VulkanDevice> device) noexcept;

private:
    void CollectAdapters() noexcept;
    void SetupDebugCallback() noexcept;
    void CreateSurface() noexcept;

    bool CheckValidationLayersSupport(ArrayIn<const AnsiChar*> validationLayers) noexcept;

private:
    NativeType m_Native;
    ObserverHandle<OwnerType> m_Owner;
    ObserverHandle<VulkanRHI> m_Ctx;

    vk::SurfaceKHR m_Surface;
    vk::DebugUtilsMessengerEXT m_Callback;
    std::vector<VulkanAdapter> m_Adapters;

    bool m_EnableValidationLayers;
    std::vector<const AnsiChar*> m_ValidationLayers;
    std::vector<const AnsiChar*> m_RequiredExtensions;
};
}
