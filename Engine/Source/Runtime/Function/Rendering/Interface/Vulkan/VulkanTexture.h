#pragma once
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanUtils.h"

namespace Snowy::Ark
{
class VulkanDevice;
class VulkanTexture
{
public:
    using NativeType = vk::Image;
    using OwnerType  = VulkanDevice;

public:
    VulkanTexture() = default;
    ~VulkanTexture() = default;
    VulkanTexture(const VulkanTexture&) = default;
    VulkanTexture(VulkanTexture&&) = default;
    VulkanTexture& operator=(const VulkanTexture&) = default;
    VulkanTexture& operator=(VulkanTexture&&) = default;

    void Init(ObserverHandle<OwnerType> owner);
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

private:
    NativeType m_Native;
    ObserverHandle<OwnerType> m_Owner;
    ObserverHandle<VulkanRHI> m_Ctx;

    vk::ImageView m_View;
};
}
