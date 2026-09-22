#pragma once

#include <cstdint>

#include <Runtime/GAL/GraphicsTypes.h>

namespace SnowyArk
{

class Buffer;
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

    /// Binds `buffer` for vertex input. `buffer` is borrowed for recording and must stay alive until the GPU finishes this frame.
    /// Requires Vertex usage, the same device, a supported binding, and offset less than GetSize(); invalid arguments throw.
    virtual void SetVertexBuffer(const Buffer& buffer, uint32_t binding = 0, uint64_t offset = 0) = 0;

    /// Binds `buffer` as the index buffer. `buffer` is borrowed for recording and must stay alive until the GPU finishes this frame.
    /// Requires Index usage, the same device, and an aligned offset with at least one complete index remaining; invalid arguments throw.
    virtual void SetIndexBuffer(const Buffer& buffer, IndexType indexType, uint64_t offset = 0) = 0;

    /// Draws `vertexCount` vertices and `instanceCount` instances starting at vertex 0.
    virtual void Draw(uint32_t vertexCount, uint32_t instanceCount) = 0;

    /// Draws `indexCount` indices and `instanceCount` instances from the bound index buffer, starting at index 0.
    /// Throws if no index buffer is bound in this recording or indexCount exceeds its remaining range. Index values must address valid vertices.
    virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount) = 0;

protected:
    CommandBuffer() = default;
};

}
