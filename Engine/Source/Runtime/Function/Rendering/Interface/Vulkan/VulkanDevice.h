#pragma once
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanUtils.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanAdapter.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanSwapchain.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanBuffer.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanTexture.h"

#include <filesystem>

namespace Snowy::Ark
{
class VulkanInstance;
class VulkanDevice
{
public:
    using NativeType = vk::Device;
    using OwnerType  = VulkanInstance;
public:
    VulkanDevice() = default;
    ~VulkanDevice() = default;
    VulkanDevice(const VulkanDevice&) = default;
    VulkanDevice(VulkanDevice&&) = default;
    VulkanDevice& operator=(const VulkanDevice&) = default;
    VulkanDevice& operator=(VulkanDevice&&) = default;

    void Init(ObserverHandle<OwnerType> owner) noexcept; 
    void Destroy() noexcept;

    auto& Native    () noexcept { return m_Native; }
    auto& Native    () const noexcept { return m_Native; }
    auto& operator* () noexcept { return m_Native; }
    auto& operator* () const noexcept { return m_Native; }
    auto* operator->() noexcept { return &m_Native; }
    auto* operator->() const noexcept { return &m_Native; }
    operator NativeType() const noexcept { return m_Native; }
    operator NativeType::NativeType() const noexcept { return m_Native; }
    ObserverHandle<OwnerType> Owner() const noexcept { return m_Owner; }
    ObserverHandle<VulkanRHI> Context() const noexcept { return m_Ctx; }

    VulkanAdapter& Adapter() const noexcept { return *m_Adapter; }
    vk::Queue& Queue(ERHIQueue type) { return m_Queues[static_cast<size_t>(type)]; }
    std::vector<const AnsiChar*>& RequiredExtensions() noexcept { return m_RequiredExtensions; }

    VulkanSwapchain CreateSwapchain() noexcept;

    VulkanBuffer CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) noexcept;
    UniqueHandle<VulkanTexture> CreateTexture(ObserverHandle<TextureData> data, ObserverHandle<TextureParams> params);
    vk::ShaderModule CreateShaderModule(ArrayIn<char> code) noexcept;

    uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) noexcept;

private:
    bool CheckDeviceExtensionSupport(In<VulkanAdapter> adapter) noexcept;
    bool IsDeviceSuitable(In<VulkanAdapter> adapter) noexcept;

private:
    NativeType m_Native;
    ObserverHandle<OwnerType> m_Owner;
    ObserverHandle<VulkanRHI> m_Ctx;

    ObserverHandle<VulkanAdapter> m_Adapter;
    std::vector<vk::Queue> m_Queues;

    std::vector<const AnsiChar*> m_RequiredExtensions;
};
}
