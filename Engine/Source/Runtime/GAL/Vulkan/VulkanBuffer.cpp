#include <Runtime/GAL/Vulkan/VulkanBuffer.h>

#include <cstring>
#include <utility>

#include <Runtime/Core/Log.h>

namespace SnowyArk
{

VulkanBuffer::VulkanBuffer(vk::raii::DeviceMemory memory, vk::raii::Buffer buffer, const uint64_t size, const BufferUsage usage)
    : Buffer(size, usage), m_Memory(std::move(memory)), m_Buffer(std::move(buffer))
{
    if (usage == BufferUsage::Uniform)
    {
        m_Mapped = m_Memory.mapMemory(0, size);
        std::memset(m_Mapped, 0, static_cast<std::size_t>(size));
    }
}

VulkanBuffer::~VulkanBuffer()
{
    if (m_Mapped != nullptr)
    {
        m_Memory.unmapMemory();
    }
}

void VulkanBuffer::Write(const std::span<const std::byte> data, const uint64_t offset)
{
    if (m_Mapped == nullptr || offset > GetSize() || data.size_bytes() > GetSize() - offset)
    {
        Log::Fatal("Buffer write requires Uniform usage and an in-range byte span.");
    }
    if (!data.empty())
    {
        std::memcpy(static_cast<std::byte*>(m_Mapped) + offset, data.data(), data.size_bytes());
    }
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
