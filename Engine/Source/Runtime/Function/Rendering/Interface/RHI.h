#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
namespace Snowy::Ark
{
enum class ERHIBackend
{
    None,
    Vulkan
};

class RHI
{
public:
    RHI() = default;
    ~RHI() = default;

    virtual void Run() = 0;
};
}


