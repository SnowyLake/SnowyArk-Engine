#pragma once

#include <cstdint>
#include <vector>

#include <Runtime/GAL/FrameBegin.h>
#include <Runtime/GAL/SwapChain.h>
#include <Runtime/GAL/Vulkan/VulkanInclude.h>

namespace SnowyArk
{

class VulkanSwapChain final : public SwapChain
{
public:
    /// Creates a swapchain for the given device, surface, and framebuffer size.
    VulkanSwapChain(vk::raii::PhysicalDevice& physicalDevice, vk::raii::Device& device, vk::raii::SurfaceKHR& surface, uint32_t graphicsQueueFamily, uint32_t presentQueueFamily, uint32_t width,
                    uint32_t height);

    Extent2D GetExtent() const override;
    Format GetColorFormat() const override;
    void Resize(uint32_t width, uint32_t height) override;

    /// Returns the native swapchain handle.
    vk::SwapchainKHR GetHandle() const;

    /// Returns the number of swapchain images.
    uint32_t GetImageCount() const;

    /// Returns the native image for `index`. Valid until RecreateIfNeeded replaces the swapchain; `index` must be less than GetImageCount().
    vk::Image GetImage(uint32_t index) const;

    /// Returns the image view for `index`. Valid until RecreateIfNeeded replaces the swapchain; `index` must be less than GetImageCount().
    vk::ImageView GetImageView(uint32_t index) const;

    /// Returns the Vulkan color format.
    vk::Format GetVkFormat() const;

    /// Returns the Vulkan extent.
    vk::Extent2D GetVkExtent() const;

    /// Acquires the next image and reports Success, Suboptimal, or OutOfDate without retrying. `imageAvailableSemaphore` must stay alive until the GPU waits on it.
    SwapChainAcquireResult AcquireNextImage(vk::Semaphore imageAvailableSemaphore);

    /// Marks the swapchain for recreation after the current acquired image is finished.
    void RequestRecreation();

    /// Recreates the swapchain when pending and drawable. Returns true when a drawable swapchain is available.
    bool RecreateIfNeeded();

private:
    /// Creates the swapchain for `width` x `height`. New images and views are committed together so members stay consistent for cleanup.
    /// Passing oldSwapchain may retire the previous swapchain even on failure; a throw is fatal for the current rendering path. Zero extent leaves recreation pending.
    void Create(uint32_t width, uint32_t height);

    /// Prefers mailbox present mode when available, otherwise FIFO.
    static vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& presentModes);

    /// Chooses a swapchain extent from surface capabilities and the requested framebuffer size.
    static vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);

    /// Chooses an image count of at least 3, clamped to the surface's supported range.
    static uint32_t ChooseImageCount(const vk::SurfaceCapabilitiesKHR& capabilities);

    vk::raii::PhysicalDevice* m_PhysicalDevice = nullptr;
    vk::raii::Device* m_Device = nullptr;
    vk::raii::SurfaceKHR* m_Surface = nullptr;
    uint32_t m_GraphicsQueueFamily = 0;
    uint32_t m_PresentQueueFamily = 0;
    vk::raii::SwapchainKHR m_Swapchain = nullptr;
    std::vector<vk::Image> m_Images;
    std::vector<vk::raii::ImageView> m_ImageViews;
    vk::Format m_Format = vk::Format::eUndefined;
    vk::Extent2D m_Extent {};
    uint32_t m_RequestedWidth = 0;
    uint32_t m_RequestedHeight = 0;
    bool m_NeedsRecreation = false;
};

}
