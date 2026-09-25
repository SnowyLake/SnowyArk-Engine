#pragma once

#include <Runtime/GAL/CommandBuffer.h>
#include <Runtime/GAL/Vulkan/VulkanInclude.h>

namespace SnowyArk
{
class VulkanPipelineState;

class VulkanCommandBuffer final : public CommandBuffer
{
public:
    /// Wraps a command buffer with its owning device and vertex binding limit.
    VulkanCommandBuffer(vk::CommandBuffer commandBuffer, vk::Device device, uint32_t maxVertexBindings, uint32_t frameIndex);

    /// Starts a fresh frame recording target and clears tracked pipeline, resource and index binding state.
    void BindTarget(vk::Image image, vk::ImageView imageView, vk::Extent2D extent);

    /// Returns the wrapped Vulkan command buffer.
    vk::CommandBuffer GetHandle() const;

    uint32_t GetFrameIndex() const override;

    void TransitionToColorTarget() override;
    void TransitionToPresent() override;
    void BeginRendering(const Color& clearColor) override;
    void EndRendering() override;
    void SetPipeline(const PipelineState& pipelineState) override;
    void SetResourceSet(const ResourceSet& resources) override;
    void SetViewport(const Viewport& viewport) override;
    void SetScissor(const Rect2D& scissor) override;
    void SetVertexBuffer(const Buffer& buffer, uint32_t binding, uint64_t offset) override;
    void SetIndexBuffer(const Buffer& buffer, IndexType indexType, uint64_t offset) override;
    void Draw(uint32_t vertexCount, uint32_t instanceCount) override;
    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount) override;

private:
    /// Rejects drawing without the current pipeline's required resource set.
    void ValidateResources() const;

    vk::Device m_Device;
    uint32_t m_MaxVertexBindings;
    uint32_t m_FrameIndex;
    const VulkanPipelineState* m_Pipeline = nullptr;
    bool m_ResourcesBound = false;
    uint64_t m_BoundIndexCount = 0;
    vk::CommandBuffer m_CommandBuffer {};
    vk::Image m_Image {};
    vk::ImageView m_ImageView {};
    vk::Extent2D m_Extent {};
};

}
