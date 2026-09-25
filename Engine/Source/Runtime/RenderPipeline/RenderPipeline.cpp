#include <Runtime/RenderPipeline/RenderPipeline.h>

#include <cstddef>
#include <span>
#include <utility>

#include <Runtime/Core/Log.h>
#include <Runtime/GAL/Buffer.h>
#include <Runtime/GAL/CommandBuffer.h>
#include <Runtime/GAL/GraphicsDevice.h>
#include <Runtime/GAL/GraphicsTypes.h>
#include <Runtime/GAL/PipelineState.h>
#include <Runtime/GAL/SwapChain.h>
#include <Runtime/RenderPipeline/RenderTransforms.h>
#include <Runtime/Shader/ShaderLibrary.h>

namespace SnowyArk
{

RenderPipeline::RenderPipeline() = default;

RenderPipeline::~RenderPipeline()
{
    Shutdown();
}

bool RenderPipeline::Initialize(GraphicsDevice& device, ShaderLibrary& shaders, const SwapChain& swapChain)
{
    if (m_Pipeline != nullptr || m_VertexBuffer != nullptr || m_IndexBuffer != nullptr || !m_Frames.empty())
    {
        return false;
    }
    static_assert(sizeof(Vertex) == 20);
    static_assert(offsetof(Vertex, position) == 0);
    static_assert(offsetof(Vertex, color) == 8);

    BufferDesc vertexDesc;
    vertexDesc.usage = BufferUsage::Vertex;
    vertexDesc.initialData = std::as_bytes(std::span { k_Vertices });
    BufferDesc indexDesc;
    indexDesc.usage = BufferUsage::Index;
    indexDesc.initialData = std::as_bytes(std::span { k_Indices });

    std::unique_ptr<Buffer> vertexBuffer = device.CreateBuffer(vertexDesc);
    std::unique_ptr<Buffer> indexBuffer = device.CreateBuffer(indexDesc);
    if (vertexBuffer == nullptr || indexBuffer == nullptr)
    {
        return false;
    }

    GraphicsPipelineDesc desc;
    desc.shaderSpirv = shaders.GetSpirv("Passes/Triangle.spv");
    desc.vertexEntryName = "MainVertex";
    desc.fragmentEntryName = "MainFragment";
    desc.colorFormat = swapChain.GetColorFormat();
    desc.vertexBindings = k_VertexBindings;
    desc.vertexAttributes = k_VertexAttributes;
    constexpr UniformBinding uniformBindings[] { { .binding = 0, .stages = ShaderStages::Vertex } };
    desc.uniformBindings = uniformBindings;
    desc.frontFace = FrontFace::CounterClockwise;
    std::unique_ptr<PipelineState> pipeline = device.CreateGraphicsPipeline(desc);
    if (pipeline == nullptr)
    {
        return false;
    }

    std::vector<FrameResources> frames;
    frames.reserve(device.GetFrameCount());
    for (uint32_t index = 0; index < device.GetFrameCount(); ++index)
    {
        auto uniforms = device.CreateBuffer({ .usage = BufferUsage::Uniform, .size = sizeof(RenderTransforms) });
        if (uniforms == nullptr)
        {
            return false;
        }
        const UniformBufferBinding binding { .binding = 0, .buffer = uniforms.get(), .size = sizeof(RenderTransforms) };
        auto resources = device.CreateResourceSet(*pipeline, std::span { &binding, 1 });
        if (resources == nullptr)
        {
            return false;
        }
        frames.push_back({ .uniforms = std::move(uniforms), .resources = std::move(resources) });
    }

    m_VertexBuffer = std::move(vertexBuffer);
    m_IndexBuffer = std::move(indexBuffer);
    m_Pipeline = std::move(pipeline);
    m_Frames = std::move(frames);
    m_StartTime = std::chrono::steady_clock::now();
    Log::Info("Created rotating indexed rectangle render pipeline with per-frame uniforms.");
    return true;
}

void RenderPipeline::Render(CommandBuffer& commandBuffer, const SwapChain& swapChain)
{
    const Extent2D extent = swapChain.GetExtent();
    const float elapsed = std::chrono::duration<float>(std::chrono::steady_clock::now() - m_StartTime).count();
    const auto transforms = RenderTransforms::Create(elapsed, extent);
    auto& frame = m_Frames.at(commandBuffer.GetFrameIndex());
    frame.uniforms->Write(std::as_bytes(std::span { &transforms, 1 }));
    const Viewport viewport { .x = 0.0f, .y = static_cast<float>(extent.height), .width = static_cast<float>(extent.width), .height = -static_cast<float>(extent.height) };
    const Rect2D scissor { .x = 0, .y = 0, .width = extent.width, .height = extent.height };
    constexpr Color clearColor { .r = 0.08f, .g = 0.08f, .b = 0.10f, .a = 1.0f };
    constexpr uint32_t indexCount = sizeof(k_Indices) / sizeof(uint16_t);

    commandBuffer.TransitionToColorTarget();
    commandBuffer.BeginRendering(clearColor);
    commandBuffer.SetPipeline(*m_Pipeline);
    commandBuffer.SetResourceSet(*frame.resources);
    commandBuffer.SetVertexBuffer(*m_VertexBuffer);
    commandBuffer.SetIndexBuffer(*m_IndexBuffer, IndexType::UInt16);
    commandBuffer.SetViewport(viewport);
    commandBuffer.SetScissor(scissor);
    commandBuffer.DrawIndexed(indexCount, 1);
    commandBuffer.EndRendering();
    commandBuffer.TransitionToPresent();
}

void RenderPipeline::Shutdown()
{
    m_Frames.clear();
    m_Pipeline.reset();
    m_VertexBuffer.reset();
    m_IndexBuffer.reset();
}

}
