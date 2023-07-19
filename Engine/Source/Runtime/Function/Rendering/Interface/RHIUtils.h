#pragma once
#include "Engine\Source\Runtime\Core\Base\Common.h"
#include "Engine//Source/Runtime/Function/Rendering/Interface/RHI.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanRHI.h"

namespace Snowy::Ark
{
class RHIUtils
{
public:
    static std::shared_ptr<RHI> CreateRHI(EGraphicsBackendType type)
    {
        if (type == EGraphicsBackendType::Vulkan)
        {
            return std::make_shared<VulkanRHI>();
        } else
        {
            return nullptr;
        }
    }
};
}
