#pragma once

#include <cstdint>

#include <Runtime/GAL/Buffer.h>
#include <Runtime/GAL/Vulkan/VulkanInclude.h>

namespace SnowyArk
{

class VulkanBuffer final : public Buffer
{
public:
    /// Takes ownership of a buffer and the device memory bound to it. `buffer` is destroyed before `memory`.
    VulkanBuffer(vk::raii::DeviceMemory memory, vk::raii::Buffer buffer, uint64_t size, BufferUsage usage);

    /// Returns the native buffer handle.
    vk::Buffer GetHandle() const;

    /// Returns the device that owns this buffer for binding validation.
    vk::Device GetDevice() const;

private:
    vk::raii::DeviceMemory m_Memory;
    vk::raii::Buffer m_Buffer;
};

}
