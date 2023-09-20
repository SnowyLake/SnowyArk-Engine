#include "VulkanBuffer.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanRHI.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanDevice.h"

namespace Snowy::Ark
{
void VulkanBuffer::Init(ObserverHandle<OwnerType> owner, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    m_Owner = owner;
    m_Ctx = owner->Context();


    vk::BufferCreateInfo bufferInfo = {
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
    };
    Utils::VerifyResult(m_Owner->Native().createBuffer(bufferInfo), STEXT("Failed to create buffer!"), &m_Native);

    vk::MemoryRequirements memRequirements;
    m_Owner->Native().getBufferMemoryRequirements(m_Native, &memRequirements);
    vk::MemoryAllocateInfo allocInfo = {
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = m_Owner->FindMemoryType(memRequirements.memoryTypeBits, properties),
    };
    Utils::VerifyResult(m_Owner->Native().allocateMemory(allocInfo), STEXT("Failed to allocate vertex buffer memory!"), &m_Memory);
    Utils::VerifyResult(m_Owner->Native().bindBufferMemory(m_Native, m_Memory, 0), STEXT("Failed to bind vertex buffer memory!"));
}
void VulkanBuffer::Destroy()
{
    m_Owner->Native().destroyBuffer(m_Native);
    m_Owner->Native().freeMemory(m_Memory);
}
}
