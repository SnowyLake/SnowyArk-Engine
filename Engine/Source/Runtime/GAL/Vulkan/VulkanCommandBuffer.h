#pragma once

#include <Runtime/GAL/CommandBuffer.h>
#include <Runtime/GAL/Vulkan/VulkanInclude.h>

namespace SnowyArk
{

class VulkanCommandBuffer final : public CommandBuffer
{
public:
    /// Wraps a command buffer allocated from the device command pool.
    explicit VulkanCommandBuffer(vk::CommandBuffer commandBuffer);

    /// Binds the swapchain image that this command buffer will render to.
    void BindTarget(vk::Image image, vk::ImageView imageView, vk::Extent2D extent);

    /// Returns the wrapped Vulkan command buffer.
    vk::CommandBuffer GetHandle() const;

    void TransitionToColorTarget() override;
    void TransitionToPresent() override;
    void BeginRendering(const Color& clearColor) override;
    void EndRendering() override;
    void SetPipeline(const PipelineState& pipelineState) override;
    void SetViewport(const Viewport& viewport) override;
    void SetScissor(const Rect2D& scissor) override;
    void Draw(uint32_t vertexCount, uint32_t instanceCount) override;

private:
    vk::CommandBuffer m_CommandBuffer {};
    vk::Image m_Image {};
    vk::ImageView m_ImageView {};
    vk::Extent2D m_Extent {};
};

}
