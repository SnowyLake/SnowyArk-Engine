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
    VulkanSwapchain() = default;
    ~VulkanSwapchain() = default;
    VulkanSwapchain(const VulkanSwapchain&) = default;
    VulkanSwapchain(VulkanSwapchain&&) = default;
    VulkanSwapchain& operator=(const VulkanSwapchain&) = default;
    VulkanSwapchain& operator=(VulkanSwapchain&&) = default;

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
    ObserverHandle<OwnerType> Owner() const noexcept { return m_Owner; }
    ObserverHandle<VulkanRHI> Context() const noexcept { return m_Ctx; }

    vk::Format ImageFormat() const noexcept { return m_ImageFormat; }
    vk::Extent2D Extent() const noexcept { return m_Extent; }
    Utils::NumType Count() const noexcept { return SA_VK_NUM(m_Images.size()); }
    const std::vector<vk::Image>& Images() const noexcept { return m_Images; }
    const std::vector<vk::ImageView>& ImageViews() const noexcept { return m_ImageViews; }
    const vk::Image& Image(size_t idx) const noexcept { return m_Images[idx]; }
    const vk::ImageView& ImageView(size_t idx) const noexcept { return m_ImageViews[idx]; }

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
