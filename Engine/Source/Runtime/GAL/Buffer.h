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
    Uniform,
};

struct BufferDesc
{
    BufferUsage usage = BufferUsage::Vertex;
    /// Borrowed only for CreateBuffer. Vertex and Index require non-empty data; Uniform may start zero-initialized.
    std::span<const std::byte> initialData;
    /// Uniform allocation size; must be nonzero. Vertex and Index derive their size from initialData.
    uint64_t size = 0;
};

class Buffer
{
public:
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    /// Destroys the buffer. The caller must already have waited out any GPU use, and the creating device must still be alive.
    virtual ~Buffer() = default;

    /// Copies bytes into a Uniform buffer. Throws for other usages or out-of-range writes.
    /// The caller must wait for all GPU reads before writing, including draws recorded but not yet submitted.
    virtual void Write(std::span<const std::byte> data, uint64_t offset = 0) = 0;

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
