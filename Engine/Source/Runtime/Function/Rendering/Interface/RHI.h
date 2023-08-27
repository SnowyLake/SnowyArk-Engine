#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Global/GlobalContextConfig.h"

#if defined(SNOWY_ARK_RHI_VULKAN)
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
class RHI
{
public:
    RHI() = default;
    ~RHI() = default;

    virtual void Init(Ref<RHIConfig> config) = 0;
    virtual void Run() = 0;
    virtual void Destory() = 0;
};
}


