#pragma once
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanUtils.h"

namespace Snowy::Ark
{
class VulkanDevice;

class VulkanBuffer
{
public:
    using NativeType = vk::Buffer;
    using OwnerType  = VulkanDevice;

public:
    VulkanBuffer() = default;
    ~VulkanBuffer() = default;
    VulkanBuffer(const VulkanBuffer&) = default;
    VulkanBuffer(VulkanBuffer&&) = default;
    VulkanBuffer& operator=(const VulkanBuffer&) = default;
    VulkanBuffer& operator=(VulkanBuffer&&) = default;

    void Init(ObserverHandle<OwnerType> owner, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
    void Destroy();

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

    const vk::DeviceMemory& Memory() const noexcept { return m_Memory; }

private:

private:
    NativeType m_Native;
    ObserverHandle<OwnerType> m_Owner;
    ObserverHandle<VulkanRHI> m_Ctx;

    vk::DeviceMemory m_Memory;
};
}
