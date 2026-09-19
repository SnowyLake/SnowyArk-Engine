#pragma once

#include <cstdint>
#include <span>

#include <Runtime/GAL/GraphicsTypes.h>

namespace SnowyArk
{

struct GraphicsPipelineDesc
{
    /// SPIR-V words borrowed for the duration of CreateGraphicsPipeline; the device copies them into a shader module.
    std::span<const uint32_t> shaderSpirv;
    const char* vertexEntryName = "vertMain";
    const char* fragmentEntryName = "fragMain";
    Format colorFormat = Format::R8G8B8A8Srgb;
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
