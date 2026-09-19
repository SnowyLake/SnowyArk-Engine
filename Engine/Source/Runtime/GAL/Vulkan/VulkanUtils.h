#pragma once

#include <span>

#include <Runtime/GAL/GraphicsTypes.h>
#include <Runtime/GAL/Vulkan/VulkanInclude.h>

namespace SnowyArk
{

class VulkanUtils
{
public:
    /// Maps an engine format to the corresponding Vulkan format.
    static vk::Format ToVkFormat(Format format);

    /// Maps a Vulkan format to the corresponding engine format.
    static Format FromVkFormat(vk::Format format);

    /// Returns true when the surface list includes R8G8B8A8Srgb with SrgbNonlinear.
    static bool HasRequiredSwapSurfaceFormat(std::span<const vk::SurfaceFormatKHR> formats);

    /// Returns the required R8G8B8A8Srgb / SrgbNonlinear surface format, or fatals if it is missing.
    static vk::SurfaceFormatKHR RequireSwapSurfaceFormat(std::span<const vk::SurfaceFormatKHR> formats);
};

}
