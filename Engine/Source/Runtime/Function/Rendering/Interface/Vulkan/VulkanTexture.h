#pragma once
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanUtils.h"

namespace Snowy::Ark
{
class VulkanDevice;
class VulkanTexture
{
public:
    using NativeType = vk::Device;
    using OwnerType  = VulkanDevice;
public:
    void Init(ObserverHandle<OwnerType> owner); 
    void Destroy();

};
}
