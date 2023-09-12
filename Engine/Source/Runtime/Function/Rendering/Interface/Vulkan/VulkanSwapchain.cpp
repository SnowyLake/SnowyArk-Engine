#include "VulkanSwapchain.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanRHI.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanDevice.h"
#include <GLFW/glfw3.h>
#undef max
#undef min

namespace Snowy::Ark
{
using Utils = VulkanUtils;

void VulkanSwapchain::Init(ObserverHandle<OwnerType> owner) noexcept
{
    m_Owner = owner;
    m_Ctx = m_Owner->GetContext();
    ResourceInit();
};

void VulkanSwapchain::Recreate() noexcept
{
    ResourceInit();
}

void VulkanSwapchain::ResourceInit() noexcept
{
    auto swapchainSupport = m_Owner->GetAdapter().QuerySwapchainSupportDetails();
    vk::SurfaceFormatKHR surfaceFormat  = ChooseSwapChainFormat(swapchainSupport.formats);
    vk::PresentModeKHR   presentMode    = ChooseSwapPresentMode(swapchainSupport.presentModes);
    vk::Extent2D         extent         = ChooseSwapExtent(swapchainSupport.capabilities);

    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
    {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo = {
        .surface = m_Owner->GetOwner()->GetSurface(),
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
    };

    QueueFamilyIndices indices = m_Owner->GetAdapter().GetQueueFamilyIndices();
    std::array<uint32_t, 2> queueFamilyIndices = { *indices.graphics, *indices.present };
    if (*indices.graphics != *indices.present)
    {
        createInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
            .setQueueFamilyIndices(queueFamilyIndices);
    } else
    {
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive)
            .setQueueFamilyIndices(nullptr);
    }

    createInfo.setPreTransform(swapchainSupport.capabilities.currentTransform)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setPresentMode(presentMode)
        .setClipped(SA_RHI_TRUE)
        .setOldSwapchain(SA_RHI_NULL);

    Utils::VerifyResult(m_Owner->Native().createSwapchainKHR(createInfo, nullptr), STEXT("Failed to create swapchain!"), &m_Native);
    Utils::VerifyResult(m_Owner->Native().getSwapchainImagesKHR(m_Native), STEXT("Failed to get swapchain images!"), &m_Images);

    m_ImageFormat = surfaceFormat.format;
    m_Extent = extent;

    m_ImageViews.resize(m_Images.size());
    for (size_t i = 0; i < m_Images.size(); i++)
    {
        vk::ImageViewCreateInfo createInfo = {
            .image = m_Images[i],
            .viewType = vk::ImageViewType::e2D,
            .format = m_ImageFormat,
            .components = vk::ComponentMapping{},
            .subresourceRange = vk::ImageSubresourceRange
            {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        Utils::VerifyResult(m_Owner->Native().createImageView(createInfo, nullptr),
                            std::format(STEXT("Failed to create swapchain image view[{}]!"), i), &m_ImageViews[i]);
    }
}

void VulkanSwapchain::Destory() noexcept
{
    for (auto&& imageView : m_ImageViews)
    {
        m_Owner->Native().destroyImageView(imageView);
    }
    m_Owner->Native().destroySwapchainKHR(m_Native);
}

vk::SurfaceFormatKHR VulkanSwapchain::ChooseSwapChainFormat(ArrayIn<vk::SurfaceFormatKHR> availableFormats) noexcept
{
    if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined)
    {
        return { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
    }

    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return availableFormat;
        }
    }
    return availableFormats[0];
}
vk::PresentModeKHR VulkanSwapchain::ChooseSwapPresentMode(ArrayIn<vk::PresentModeKHR> availablePresentModes) noexcept
{
    auto bestMode = vk::PresentModeKHR::eFifo;
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox)
        {
            return availablePresentMode;
        } else
        {
            bestMode = vk::PresentModeKHR::eImmediate;
        }
    }
    return bestMode;
}
vk::Extent2D VulkanSwapchain::ChooseSwapExtent(In<vk::SurfaceCapabilitiesKHR> capabilities) noexcept
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    } else
    {
        int actualWidth, actualHeight;
        glfwGetFramebufferSize(m_Ctx->GetWindowHandle(), &actualWidth, &actualHeight);
        vk::Extent2D actualExtent = { static_cast<uint32_t>(actualWidth), static_cast<uint32_t>(actualHeight) };
        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }
}
}
