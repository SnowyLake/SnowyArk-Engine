#pragma once
#include "Engine/Source/Runtime/Function/Rendering/Interface/RHI.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanRHI.h"

namespace Snowy::Ark
{
struct RHIConfig
{
    ERHIBackend backend;
};

class RHIManager
{
public:
    static SharedHandle<RHI> CreateRHI(RHIConfig config)
    {
        if (config.backend == ERHIBackend::Vulkan)
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