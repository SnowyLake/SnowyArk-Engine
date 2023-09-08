#pragma once
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanUtils.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanDevice.h"

namespace Snowy::Ark
{
class VulkanTexture
{
public:
    using NativeType = vk::Device;
    using OwnerType  = VulkanInstance;
public:
    void Init(ObserverHandle<OwnerType> owner); 
    void Destroy();

};
}
