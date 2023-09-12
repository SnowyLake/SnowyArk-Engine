#pragma once
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanUtils.h"

namespace Snowy::Ark
{
using Utils = VulkanUtils;
class VulkanDevice;
class VulkanSwapchain
{
public:
    using NativeType = vk::SwapchainKHR;
    using OwnerType  = VulkanDevice;

public:
    void Init(ObserverHandle<OwnerType> owner) noexcept;
    void Destory() noexcept;

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

    vk::Format GetImageFormat() const noexcept { return m_ImageFormat; }
    vk::Extent2D GetExtent() const noexcept { return m_Extent; }
    Utils::NumType GetImageCount() const noexcept { return Utils::CastNumType(m_Images.size()); }
    const std::vector<vk::Image>& GetImages() const noexcept { return m_Images; }
    const std::vector<vk::ImageView>& GetImageViews() const noexcept { return m_ImageViews; }
    const vk::Image& GetImage(size_t idx) const noexcept { return m_Images[idx]; }
    const vk::ImageView& GetImageView(size_t idx) const noexcept { return m_ImageViews[idx]; }

    void Recreate() noexcept;

private:
    void ResourceInit() noexcept;
    vk::SurfaceFormatKHR ChooseSwapChainFormat(ArrayIn<vk::SurfaceFormatKHR> availableFormats) noexcept;
    vk::PresentModeKHR ChooseSwapPresentMode(ArrayIn<vk::PresentModeKHR> availablePresentModes) noexcept;
    vk::Extent2D ChooseSwapExtent(In<vk::SurfaceCapabilitiesKHR> capabilities) noexcept;

private:
    NativeType m_Native;
    ObserverHandle<OwnerType> m_Owner;
    ObserverHandle<VulkanRHI> m_Ctx;

    vk::Format m_ImageFormat;
    vk::Extent2D m_Extent;
    std::vector<vk::Image> m_Images;
    std::vector<vk::ImageView> m_ImageViews;
};
}
