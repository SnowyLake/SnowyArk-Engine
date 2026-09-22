#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace SnowyArk
{

enum class BufferUsage
{
    Vertex,
    Index,
};

struct BufferDesc
{
    BufferUsage usage = BufferUsage::Vertex;
    /// Borrowed only for CreateBuffer. Copied into device-local memory before the call returns. Must be non-empty.
    std::span<const std::byte> initialData;
};

class Buffer
{
public:
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    /// Destroys the buffer. The caller must already have waited out any GPU use, and the creating device must still be alive.
    virtual ~Buffer() = default;

    /// Returns the buffer's byte size, excluding allocation padding.
    uint64_t GetSize() const
    {
        return m_Size;
    }

    /// Returns the immutable binding usage selected at creation.
    BufferUsage GetUsage() const
    {
        return m_Usage;
    }

protected:
    /// Stores backend-independent metadata for a fully allocated buffer.
    Buffer(const uint64_t size, const BufferUsage usage)
        : m_Size(size), m_Usage(usage)
    {
    }

private:
    uint64_t m_Size;
    BufferUsage m_Usage;
};

}
