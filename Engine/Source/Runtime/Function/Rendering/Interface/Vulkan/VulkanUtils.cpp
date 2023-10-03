#include "VulkanUtils.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanRHI.h"


#include <iostream>
#include <format>

namespace Snowy::Ark
{
bool VulkanUtils::HasStencilComponent(vk::Format format)
{
    using enum vk::Format;
    return format == eD32SfloatS8Uint || format == eD24UnormS8Uint;
}
}
