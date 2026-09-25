#pragma once

#include <cstdint>

namespace SnowyArk
{
class Buffer;

struct UniformBufferBinding
{
    uint32_t binding = 0;
    const Buffer* buffer = nullptr;
    uint64_t offset = 0;
    uint64_t size = 0;
};

class ResourceSet
{
public:
    ResourceSet(const ResourceSet&) = delete;
    ResourceSet& operator=(const ResourceSet&) = delete;

    /// Releases immutable bindings after GPU completion. Referenced buffers and creating pipeline must outlive this set.
    virtual ~ResourceSet() = default;

protected:
    ResourceSet() = default;
};
}
