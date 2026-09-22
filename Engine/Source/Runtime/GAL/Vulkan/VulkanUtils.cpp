#include <Runtime/GAL/Vulkan/VulkanUtils.h>

#include <algorithm>

#include <Runtime/Core/Log.h>

namespace SnowyArk
{

vk::Format VulkanUtils::ToVkFormat(const Format format)
{
    switch (format)
    {
    case Format::R8G8B8A8Srgb:
        return vk::Format::eR8G8B8A8Srgb;
    case Format::R32G32Sfloat:
        return vk::Format::eR32G32Sfloat;
    case Format::R32G32B32Sfloat:
        return vk::Format::eR32G32B32Sfloat;
    case Format::Unknown:
        break;
    }
    Log::Fatal("Unsupported graphics format.");
}

Format VulkanUtils::FromVkFormat(const vk::Format format)
{
    switch (format)
    {
    case vk::Format::eR8G8B8A8Srgb:
        return Format::R8G8B8A8Srgb;
    default:
        Log::Fatal("Surface does not support R8G8B8A8Srgb with SrgbNonlinear.");
    }
}

bool VulkanUtils::HasRequiredSwapSurfaceFormat(const std::span<const vk::SurfaceFormatKHR> formats)
{
    return std::ranges::any_of(formats, [](const vk::SurfaceFormatKHR& format) { return format.format == vk::Format::eR8G8B8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
}

vk::SurfaceFormatKHR VulkanUtils::RequireSwapSurfaceFormat(const std::span<const vk::SurfaceFormatKHR> formats)
{
    const auto found =
        std::ranges::find_if(formats, [](const vk::SurfaceFormatKHR& format) { return format.format == vk::Format::eR8G8B8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
    if (found == formats.end())
    {
        Log::Fatal("Surface does not support R8G8B8A8Srgb with SrgbNonlinear.");
    }
    return *found;
}

}
