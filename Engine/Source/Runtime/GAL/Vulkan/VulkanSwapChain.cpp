#include <Runtime/GAL/Vulkan/VulkanSwapChain.h>

#include <algorithm>
#include <limits>
#include <utility>
#include <vector>

#include <Runtime/Core/Assert.h>
#include <Runtime/Core/Log.h>
#include <Runtime/GAL/Vulkan/VulkanUtils.h>

namespace SnowyArk
{

VulkanSwapChain::VulkanSwapChain(vk::raii::PhysicalDevice& physicalDevice, vk::raii::Device& device, vk::raii::SurfaceKHR& surface, const uint32_t graphicsQueueFamily,
                                 const uint32_t presentQueueFamily, const uint32_t width, const uint32_t height)
    : m_PhysicalDevice(&physicalDevice),
      m_Device(&device),
      m_Surface(&surface),
      m_GraphicsQueueFamily(graphicsQueueFamily),
      m_PresentQueueFamily(presentQueueFamily),
      m_RequestedWidth(width),
      m_RequestedHeight(height),
      m_NeedsRecreation(true)
{
    RecreateIfNeeded();
}

Extent2D VulkanSwapChain::GetExtent() const
{
    return { m_Extent.width, m_Extent.height };
}

Format VulkanSwapChain::GetColorFormat() const
{
    return VulkanUtils::FromVkFormat(m_Format);
}

void VulkanSwapChain::Resize(const uint32_t width, const uint32_t height)
{
    m_RequestedWidth = width;
    m_RequestedHeight = height;
    m_NeedsRecreation = true;
    RecreateIfNeeded();
}

vk::SwapchainKHR VulkanSwapChain::GetHandle() const
{
    return *m_Swapchain;
}

uint32_t VulkanSwapChain::GetImageCount() const
{
    SNOWYARK_ASSERT(m_Images.size() <= std::numeric_limits<uint32_t>::max());
    return static_cast<uint32_t>(m_Images.size());
}

vk::Image VulkanSwapChain::GetImage(const uint32_t index) const
{
    SNOWYARK_ASSERT(index < m_Images.size());
    return m_Images[index];
}

vk::ImageView VulkanSwapChain::GetImageView(const uint32_t index) const
{
    SNOWYARK_ASSERT(index < m_ImageViews.size());
    return *m_ImageViews[index];
}

vk::Format VulkanSwapChain::GetVkFormat() const
{
    return m_Format;
}

vk::Extent2D VulkanSwapChain::GetVkExtent() const
{
    return m_Extent;
}

SwapChainAcquireResult VulkanSwapChain::AcquireNextImage(const vk::Semaphore imageAvailableSemaphore)
{
    try
    {
        const vk::ResultValue<uint32_t> result = m_Swapchain.acquireNextImage(UINT64_MAX, imageAvailableSemaphore, nullptr);
        if (result.result == vk::Result::eSuboptimalKHR)
        {
            return { .status = SwapChainAcquireStatus::Suboptimal, .imageIndex = result.value };
        }
        return { .status = SwapChainAcquireStatus::Success, .imageIndex = result.value };
    }
    catch (const vk::OutOfDateKHRError&)
    {
        return { .status = SwapChainAcquireStatus::OutOfDate, .imageIndex = 0 };
    }
}

void VulkanSwapChain::RequestRecreation()
{
    m_NeedsRecreation = true;
}

bool VulkanSwapChain::RecreateIfNeeded()
{
    if (!m_NeedsRecreation)
    {
        return m_Swapchain != nullptr && m_Extent.width > 0 && m_Extent.height > 0;
    }
    if (m_RequestedWidth == 0 || m_RequestedHeight == 0)
    {
        return false;
    }

    m_Device->waitIdle();
    Create(m_RequestedWidth, m_RequestedHeight);
    return !m_NeedsRecreation && m_Swapchain != nullptr && m_Extent.width > 0 && m_Extent.height > 0;
}

void VulkanSwapChain::Create(const uint32_t width, const uint32_t height)
{
    const vk::SurfaceCapabilitiesKHR capabilities = m_PhysicalDevice->getSurfaceCapabilitiesKHR(**m_Surface);
    const vk::SurfaceFormatKHR surfaceFormat = VulkanUtils::RequireSwapSurfaceFormat(m_PhysicalDevice->getSurfaceFormatsKHR(**m_Surface));
    const vk::PresentModeKHR presentMode = ChooseSwapPresentMode(m_PhysicalDevice->getSurfacePresentModesKHR(**m_Surface));
    const vk::Extent2D extent = ChooseSwapExtent(capabilities, width, height);
    if (extent.width == 0 || extent.height == 0)
    {
        m_NeedsRecreation = true;
        return;
    }

    const uint32_t queueFamilies[] = { m_GraphicsQueueFamily, m_PresentQueueFamily };
    const bool concurrent = m_GraphicsQueueFamily != m_PresentQueueFamily;

    const vk::SwapchainCreateInfoKHR createInfo {
        .surface = **m_Surface,
        .minImageCount = ChooseImageCount(capabilities),
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = concurrent ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = concurrent ? 2u : 0u,
        .pQueueFamilyIndices = concurrent ? queueFamilies : nullptr,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode,
        .clipped = vk::True,
        .oldSwapchain = (m_Swapchain != nullptr) ? *m_Swapchain : vk::SwapchainKHR {},
    };

    vk::raii::SwapchainKHR swapchain(*m_Device, createInfo);
    std::vector<vk::Image> images = swapchain.getImages();
    std::vector<vk::raii::ImageView> imageViews;
    imageViews.reserve(images.size());
    for (const vk::Image image : images)
    {
        const vk::ImageViewCreateInfo viewInfo {
            .image = image,
            .viewType = vk::ImageViewType::e2D,
            .format = surfaceFormat.format,
            .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
        };
        imageViews.emplace_back(*m_Device, viewInfo);
    }

    m_ImageViews.clear();
    m_Images.clear();
    m_Swapchain = std::move(swapchain);
    m_Images = std::move(images);
    m_ImageViews = std::move(imageViews);
    m_Extent = extent;
    m_Format = surfaceFormat.format;
    m_NeedsRecreation = false;
    Log::Info("Created swapchain ({}x{}, {} images, format {}).", m_Extent.width, m_Extent.height, m_Images.size(), static_cast<int32_t>(m_Format));
}

vk::PresentModeKHR VulkanSwapChain::ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& presentModes)
{
    if (const auto found = std::ranges::find(presentModes, vk::PresentModeKHR::eMailbox); found != presentModes.end())
    {
        return *found;
    }
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D VulkanSwapChain::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const uint32_t width, const uint32_t height)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }

    vk::Extent2D extent { width, height };
    extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return extent;
}

uint32_t VulkanSwapChain::ChooseImageCount(const vk::SurfaceCapabilitiesKHR& capabilities)
{
    uint32_t imageCount = std::max(3u, capabilities.minImageCount);
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
    {
        imageCount = capabilities.maxImageCount;
    }
    return imageCount;
}

}
