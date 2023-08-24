#pragma once
#include "Engine/Source/Runtime/Function/Rendering/Interface/RHI.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanRHI.h"

namespace Snowy::Ark
{
class RHIManager
{
public:
    static SharedHandle<RHI> CreateRHI(ERHIBackend backend)
    {
        if (backend == ERHIBackend::Vulkan)
        {
            return MakeShared<VulkanRHI>();
        } else
        {
            return nullptr;
        }
    }
    static void DestroyRHI(SharedHandle<RHI> rhi)
    {
        rhi->Destory();
    }
};
}