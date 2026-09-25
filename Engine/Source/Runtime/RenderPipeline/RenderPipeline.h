#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include <Runtime/GAL/Buffer.h>
#include <Runtime/GAL/PipelineState.h>
#include <Runtime/GAL/ResourceSet.h>

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

    /// Creates the indexed rectangle pipeline, geometry, and independent uniform resources for each in-flight slot.
    /// Returns false without replacing live resources when already initialized. Call Shutdown after GPU completion before reinitializing.
    [[nodiscard]] bool Initialize(GraphicsDevice& device, ShaderLibrary& shaders, const SwapChain& swapChain);

    /// Updates the completed frame slot's MVP and records the rotating rectangle; call once per successful BeginFrame.
    /// Both arguments are borrowed for this call only. No allocations or extra GPU waits occur here.
    void Render(CommandBuffer& commandBuffer, const SwapChain& swapChain);

    /// Releases pipeline and buffer resources. The creating device must still be alive, and GPU use must already have finished.
    void Shutdown();

private:
    struct FrameResources
    {
        std::unique_ptr<Buffer> uniforms;
        std::unique_ptr<ResourceSet> resources;
    };
    struct Vertex
    {
        float position[2];
        float color[3];
    };

    static constexpr Vertex k_Vertices[] {
        { .position = { -0.5f, -0.5f }, .color = { 1.0f, 0.0f, 0.0f } },
        { .position = { 0.5f, -0.5f }, .color = { 0.0f, 1.0f, 0.0f } },
        { .position = { 0.5f, 0.5f }, .color = { 0.0f, 0.0f, 1.0f } },
        { .position = { -0.5f, 0.5f }, .color = { 1.0f, 1.0f, 1.0f } },
    };
    static constexpr uint16_t k_Indices[] { 0, 1, 2, 2, 3, 0 };
    static constexpr VertexBinding k_VertexBindings[] {
        { .binding = 0, .stride = sizeof(Vertex), .inputRate = VertexInputRate::PerVertex },
    };
    static constexpr VertexAttribute k_VertexAttributes[] {
        { .location = 0, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(Vertex, position) },
        { .location = 1, .binding = 0, .format = Format::R32G32B32Sfloat, .offset = offsetof(Vertex, color) },
    };

    std::unique_ptr<PipelineState> m_Pipeline;
    std::unique_ptr<Buffer> m_VertexBuffer;
    std::unique_ptr<Buffer> m_IndexBuffer;
    std::vector<FrameResources> m_Frames;
    std::chrono::steady_clock::time_point m_StartTime;
};

}
