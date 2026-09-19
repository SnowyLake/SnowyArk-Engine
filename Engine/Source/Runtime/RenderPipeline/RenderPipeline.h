#pragma once

#include <memory>

#include <Runtime/GAL/PipelineState.h>

namespace SnowyArk
{

class CommandBuffer;
class GraphicsDevice;
class ShaderLibrary;
class SwapChain;

class RenderPipeline
{
public:
    RenderPipeline();
    RenderPipeline(const RenderPipeline&) = delete;
    RenderPipeline& operator=(const RenderPipeline&) = delete;
    ~RenderPipeline();

    /// Creates the hardcoded triangle pipeline.
    [[nodiscard]] bool Initialize(GraphicsDevice& device, ShaderLibrary& shaders, const SwapChain& swapChain);

    /// Records the hardcoded triangle pass into `commandBuffer`. Both arguments are borrowed for this call only.
    void Render(CommandBuffer& commandBuffer, const SwapChain& swapChain);

    /// Releases pipeline resources.
    void Shutdown();

private:
    std::unique_ptr<PipelineState> m_TrianglePipeline;
};

}
