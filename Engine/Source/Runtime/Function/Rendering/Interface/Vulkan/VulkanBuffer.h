#pragma once
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanUtils.h"

namespace Snowy::Ark
{
class VulkanDevice;

class VulkanBuffer
{
public:
    using NativeType = vk::Buffer;
    using OwnerType  = VulkanDevice;
public:
    void Init(ObserverHandle<OwnerType> owner, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
    void Destroy();
private:

private:
    NativeType m_Native;
    ObserverHandle<OwnerType> m_Owner;
    ObserverHandle<VulkanRHI> m_Ctx;

    vk::DeviceMemory m_Memory;
};
}
