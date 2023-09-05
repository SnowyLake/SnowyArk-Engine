#include "RHI.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanRHI.h"

namespace Snowy::Ark
{
SharedHandle<RHI> RHI::CreateRHI(ERHIBackend backend)
{
    if (backend == ERHIBackend::Vulkan)
    {
        return MakeShared<VulkanRHI>();
    } else
    {
        return nullptr;
    }
}
void RHI::DestroyRHI(SharedHandle<RHI> rhi)
{
    rhi->Destory();
}
}
