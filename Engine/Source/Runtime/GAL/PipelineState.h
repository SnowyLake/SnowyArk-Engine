#pragma once

#include <cstdint>
#include <span>

#include <Runtime/GAL/GraphicsTypes.h>

namespace SnowyArk
{

struct VertexBinding
{
    uint32_t binding = 0;
    uint32_t stride = 0;
    VertexInputRate inputRate = VertexInputRate::PerVertex;
};

struct VertexAttribute
{
    uint32_t location = 0;
    uint32_t binding = 0;
    Format format = Format::Unknown;
    uint32_t offset = 0;
};

struct GraphicsPipelineDesc
{
    /// SPIR-V words borrowed for the duration of CreateGraphicsPipeline; the device copies them into a shader module.
    std::span<const uint32_t> shaderSpirv;
    const char* vertexEntryName = "MainVertex";
    const char* fragmentEntryName = "MainFragment";
    Format colorFormat = Format::R8G8B8A8Srgb;
    /// Borrowed for the duration of CreateGraphicsPipeline. Empty means the pipeline reads no vertex buffers.
    std::span<const VertexBinding> vertexBindings;
    /// Borrowed for the duration of CreateGraphicsPipeline. Locations must match the vertex shader inputs.
    std::span<const VertexAttribute> vertexAttributes;
};

class PipelineState
{
public:
    PipelineState(const PipelineState&) = delete;
    PipelineState& operator=(const PipelineState&) = delete;
    virtual ~PipelineState() = default;

protected:
    PipelineState() = default;
};

}
