#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Global/GlobalContextConfig.h"

#if defined(SNOWY_ARK_RHI_VULKAN)
#define SA_RHI_TRUE     VK_TRUE
#define SA_RHI_FALSE    VK_FALSE
#define SA_RHI_NULL     VK_NULL_HANDLE
#else
#define SA_RHI_TRUE     true
#define SA_RHI_FALSE    false
#define SA_RHI_NULL     nullptr
#endif

namespace Snowy::Ark
{
class RHI
{
public:
    RHI() = default;
    virtual ~RHI() = default;
    RHI(const RHI&) = default;
    RHI(RHI&&) = default;
    RHI& operator=(const RHI&) = default;
    RHI& operator=(RHI&&) = default;

    virtual void Init(In<RHIConfig> config) = 0;
    virtual void Run() = 0;
    virtual void Destory() = 0;

public:
    static SharedHandle<RHI> CreateRHI(ERHIBackend backend);
    static void DestroyRHI(SharedHandle<RHI> rhi);
};
}
