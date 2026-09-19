#include <Runtime/GAL/Vulkan/VulkanCommandBuffer.h>

#include <Runtime/GAL/Vulkan/VulkanPipelineState.h>

namespace SnowyArk
{

VulkanCommandBuffer::VulkanCommandBuffer(const vk::CommandBuffer commandBuffer)
    : m_CommandBuffer(commandBuffer)
{
}

void VulkanCommandBuffer::BindTarget(const vk::Image image, const vk::ImageView imageView, const vk::Extent2D extent)
{
    m_Image = image;
    m_ImageView = imageView;
    m_Extent = extent;
}

vk::CommandBuffer VulkanCommandBuffer::GetHandle() const
{
    return m_CommandBuffer;
}

void VulkanCommandBuffer::TransitionToColorTarget()
{
    const vk::ImageMemoryBarrier2 barrier {
        .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .srcAccessMask = vk::AccessFlagBits2::eNone,
        .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
        .oldLayout = vk::ImageLayout::eUndefined,
        .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .image = m_Image,
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    const vk::DependencyInfo dependency { .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrier };
    m_CommandBuffer.pipelineBarrier2(dependency);
}

void VulkanCommandBuffer::TransitionToPresent()
{
    const vk::ImageMemoryBarrier2 barrier {
        .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
        .dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,
        .dstAccessMask = vk::AccessFlagBits2::eNone,
        .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .newLayout = vk::ImageLayout::ePresentSrcKHR,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .image = m_Image,
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    const vk::DependencyInfo dependency { .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrier };
    m_CommandBuffer.pipelineBarrier2(dependency);
}

void VulkanCommandBuffer::BeginRendering(const Color& clearColor)
{
    const vk::ClearColorValue clearValue { clearColor.r, clearColor.g, clearColor.b, clearColor.a };
    const vk::RenderingAttachmentInfo colorAttachment {
        .imageView = m_ImageView,
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearValue,
    };
    const vk::RenderingInfo renderingInfo {
        .renderArea = {
            .offset = { .x = 0, .y = 0 },
            .extent = m_Extent,
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment,
    };
    m_CommandBuffer.beginRendering(renderingInfo);
}

void VulkanCommandBuffer::EndRendering()
{
    m_CommandBuffer.endRendering();
}

void VulkanCommandBuffer::SetPipeline(const PipelineState& pipelineState)
{
    const auto& vulkanPipeline = dynamic_cast<const VulkanPipelineState&>(pipelineState);
    m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, vulkanPipeline.GetPipeline());
}

void VulkanCommandBuffer::SetViewport(const Viewport& viewport)
{
    const vk::Viewport vkViewport {
        .x = viewport.x,
        .y = viewport.y,
        .width = viewport.width,
        .height = viewport.height,
        .minDepth = viewport.minDepth,
        .maxDepth = viewport.maxDepth,
    };
    m_CommandBuffer.setViewport(0, vkViewport);
}

void VulkanCommandBuffer::SetScissor(const Rect2D& scissor)
{
    const vk::Rect2D vkScissor {
        .offset = { .x = scissor.x, .y = scissor.y },
        .extent = { .width = scissor.width, .height = scissor.height },
    };
    m_CommandBuffer.setScissor(0, vkScissor);
}

void VulkanCommandBuffer::Draw(const uint32_t vertexCount, const uint32_t instanceCount)
{
    m_CommandBuffer.draw(vertexCount, instanceCount, 0, 0);
}

}
