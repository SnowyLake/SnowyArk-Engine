#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"

#if defined(RHI_VULKAN)
#define RHI_TRUE            VK_TRUE
#define RHI_FALSE           VK_FALSE
#define RHI_NULL_HANDLE     VK_NULL_HANDLE
#else
#define RHI_TRUE            true
#define RHI_FALSE           false
#define RHI_NULL_HANDLE     nullptr
#endif

namespace Snowy::Ark
{
enum class ERHIBackend
{
    None,
    // OpenGL,
    Vulkan,
    // DirectX11,
    // DirectX12,
};

class RHI
{
public:
    RHI() = default;
    ~RHI() = default;

    virtual void Run() = 0;
    virtual void Cleanup() = 0;
};
}


