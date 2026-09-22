#include <Runtime/GAL/Vulkan/VulkanBuffer.h>

#include <utility>

namespace SnowyArk
{

VulkanBuffer::VulkanBuffer(vk::raii::DeviceMemory memory, vk::raii::Buffer buffer, const uint64_t size, const BufferUsage usage)
    : Buffer(size, usage), m_Memory(std::move(memory)), m_Buffer(std::move(buffer))
{
}

vk::Buffer VulkanBuffer::GetHandle() const
{
    return *m_Buffer;
}

vk::Device VulkanBuffer::GetDevice() const
{
    return m_Buffer.getDevice();
}

}
