#pragma once
#include "Engine/Source/Runtime/Function/Rendering/Interface/RHITexture.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanUtils.h"

namespace Snowy::Ark
{
struct VulkanTextureParams
{
    vk::ImageType type;
    vk::Format format;
    vk::ImageTiling tiling;
    vk::ImageUsageFlags usage;
    vk::MemoryPropertyFlags memoryProps;

    vk::ImageViewType viewType;
    vk::ImageAspectFlagBits aspectMask;
};

class VulkanDevice;
class VulkanTexture : public RHITexture
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

    void Init(ObserverHandle<OwnerType> owner, In<TextureData> data, In<VulkanTextureParams> params);
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
    const vk::ImageView& View() const noexcept { return m_View; }
    const vk::Sampler& Sampler() const noexcept { return m_Sampler; }

    void TransitionLayout(vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

private:
    NativeType m_Native;
    ObserverHandle<OwnerType> m_Owner;
    ObserverHandle<VulkanRHI> m_Ctx;

    vk::DeviceMemory m_Memory;
    vk::ImageView m_View;
    vk::Sampler m_Sampler;
};
}
