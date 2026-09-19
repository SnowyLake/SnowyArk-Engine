#pragma once

#include <cstdint>

#include <Runtime/GAL/GraphicsTypes.h>

namespace SnowyArk
{

class PipelineState;

class CommandBuffer
{
public:
    CommandBuffer(const CommandBuffer&) = delete;
    CommandBuffer& operator=(const CommandBuffer&) = delete;
    virtual ~CommandBuffer() = default;

    /// Transitions the current swapchain image to a color attachment layout.
    virtual void TransitionToColorTarget() = 0;

    /// Transitions the current swapchain image to the present layout.
    virtual void TransitionToPresent() = 0;

    /// Begins a dynamic rendering pass that targets the current swapchain image.
    virtual void BeginRendering(const Color& clearColor) = 0;

    /// Ends the current dynamic rendering pass.
    virtual void EndRendering() = 0;

    /// Binds a graphics pipeline for subsequent draw calls.
    virtual void SetPipeline(const PipelineState& pipelineState) = 0;

    /// Sets the dynamic viewport.
    virtual void SetViewport(const Viewport& viewport) = 0;

    /// Sets the dynamic scissor rectangle.
    virtual void SetScissor(const Rect2D& scissor) = 0;

    /// Draws `vertexCount` vertices and `instanceCount` instances starting at vertex 0.
    virtual void Draw(uint32_t vertexCount, uint32_t instanceCount) = 0;

protected:
    CommandBuffer() = default;
};

}