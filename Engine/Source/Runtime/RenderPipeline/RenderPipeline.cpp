#include <Runtime/RenderPipeline/RenderPipeline.h>

#include <memory>

#include <Runtime/Core/Log.h>
#include <Runtime/GAL/CommandBuffer.h>
#include <Runtime/GAL/GraphicsDevice.h>
#include <Runtime/GAL/GraphicsTypes.h>
#include <Runtime/GAL/PipelineState.h>
#include <Runtime/GAL/SwapChain.h>
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
    GraphicsPipelineDesc desc;
    desc.shaderSpirv = shaders.GetSpirv("Passes/Triangle.spv");
    desc.vertexEntryName = "vertMain";
    desc.fragmentEntryName = "fragMain";
    desc.colorFormat = swapChain.GetColorFormat();
    m_TrianglePipeline = device.CreateGraphicsPipeline(desc);
    Log::Info("Created triangle render pipeline.");
    return m_TrianglePipeline != nullptr;
}

void RenderPipeline::Render(CommandBuffer& commandBuffer, const SwapChain& swapChain)
{
    const Extent2D extent = swapChain.GetExtent();
    const Viewport viewport { .x = 0.0f, .y = 0.0f, .width = static_cast<float>(extent.width), .height = static_cast<float>(extent.height) };
    const Rect2D scissor { .x = 0, .y = 0, .width = extent.width, .height = extent.height };
    constexpr Color clearColor { .r = 0.08f, .g = 0.08f, .b = 0.10f, .a = 1.0f };

    commandBuffer.TransitionToColorTarget();
    commandBuffer.BeginRendering(clearColor);
    commandBuffer.SetPipeline(*m_TrianglePipeline);
    commandBuffer.SetViewport(viewport);
    commandBuffer.SetScissor(scissor);
    commandBuffer.Draw(3, 1);
    commandBuffer.EndRendering();
    commandBuffer.TransitionToPresent();
}

void RenderPipeline::Shutdown()
{
    m_TrianglePipeline.reset();
}

}
