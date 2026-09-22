#pragma once

#include <Runtime/GAL/CommandBuffer.h>
#include <Runtime/GAL/Vulkan/VulkanInclude.h>

namespace SnowyArk
{

class VulkanCommandBuffer final : public CommandBuffer
{
public:
    /// Wraps a command buffer with its owning device and vertex binding limit.
    VulkanCommandBuffer(vk::CommandBuffer commandBuffer, vk::Device device, uint32_t maxVertexBindings);

    /// Starts a fresh frame recording target and clears tracked index binding state.
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
    void SetVertexBuffer(const Buffer& buffer, uint32_t binding, uint64_t offset) override;
    void SetIndexBuffer(const Buffer& buffer, IndexType indexType, uint64_t offset) override;
    void Draw(uint32_t vertexCount, uint32_t instanceCount) override;
    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount) override;

private:
    vk::Device m_Device;
    uint32_t m_MaxVertexBindings;
    uint64_t m_BoundIndexCount = 0;
    vk::CommandBuffer m_CommandBuffer {};
    vk::Image m_Image {};
    vk::ImageView m_ImageView {};
    vk::Extent2D m_Extent {};
};

}
