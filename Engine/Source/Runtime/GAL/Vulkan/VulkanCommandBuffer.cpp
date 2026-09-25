#include <Runtime/GAL/Vulkan/VulkanCommandBuffer.h>

#include <Runtime/Core/Log.h>
#include <Runtime/GAL/Vulkan/VulkanBuffer.h>
#include <Runtime/GAL/Vulkan/VulkanPipelineState.h>
#include <Runtime/GAL/Vulkan/VulkanResourceSet.h>

namespace SnowyArk
{

VulkanCommandBuffer::VulkanCommandBuffer(const vk::CommandBuffer commandBuffer, const vk::Device device, const uint32_t maxVertexBindings, const uint32_t frameIndex)
    : m_Device(device), m_MaxVertexBindings(maxVertexBindings), m_FrameIndex(frameIndex), m_CommandBuffer(commandBuffer)
{
}

void VulkanCommandBuffer::BindTarget(const vk::Image image, const vk::ImageView imageView, const vk::Extent2D extent)
{
    m_Image = image;
    m_ImageView = imageView;
    m_Extent = extent;
    m_BoundIndexCount = 0;
    m_Pipeline = nullptr;
    m_ResourcesBound = false;
}

uint32_t VulkanCommandBuffer::GetFrameIndex() const
{
    return m_FrameIndex;
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
    const auto* vulkanPipeline = dynamic_cast<const VulkanPipelineState*>(&pipelineState);
    if (vulkanPipeline == nullptr || vulkanPipeline->GetDevice() != m_Device)
    {
        Log::Fatal("Pipeline must belong to this command buffer's device.");
    }
    m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, vulkanPipeline->GetPipeline());
    m_Pipeline = vulkanPipeline;
    m_ResourcesBound = false;
}

void VulkanCommandBuffer::SetResourceSet(const ResourceSet& resources)
{
    const auto* set = dynamic_cast<const VulkanResourceSet*>(&resources);
    if (set == nullptr || m_Pipeline == nullptr || &set->GetPipeline() != m_Pipeline)
    {
        Log::Fatal("Resource set must be bound with its creating pipeline.");
    }
    m_CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_Pipeline->GetLayout(), 0, set->GetHandle(), {});
    m_ResourcesBound = true;
}

void VulkanCommandBuffer::ValidateResources() const
{
    if (m_Pipeline == nullptr || (!m_Pipeline->GetUniformBindings().empty() && !m_ResourcesBound))
    {
        Log::Fatal("Draw requires a pipeline and all declared uniform bindings.");
    }
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

void VulkanCommandBuffer::SetVertexBuffer(const Buffer& buffer, const uint32_t binding, const uint64_t offset)
{
    const auto& vulkanBuffer = dynamic_cast<const VulkanBuffer&>(buffer);
    if (vulkanBuffer.GetDevice() != m_Device || buffer.GetUsage() != BufferUsage::Vertex || binding >= m_MaxVertexBindings || offset >= buffer.GetSize())
    {
        Log::Fatal("Vertex buffer binding requires the same device, Vertex usage, a supported binding, and an in-range offset.");
    }
    const vk::DeviceSize vkOffset = offset;
    m_CommandBuffer.bindVertexBuffers(binding, vulkanBuffer.GetHandle(), vkOffset);
}

void VulkanCommandBuffer::SetIndexBuffer(const Buffer& buffer, const IndexType indexType, const uint64_t offset)
{
    const auto& vulkanBuffer = dynamic_cast<const VulkanBuffer&>(buffer);
    if (vulkanBuffer.GetDevice() != m_Device || buffer.GetUsage() != BufferUsage::Index || offset >= buffer.GetSize())
    {
        Log::Fatal("Index buffer binding requires the same device, Index usage, and an in-range offset.");
    }

    vk::IndexType vkIndexType = vk::IndexType::eUint16;
    uint64_t indexSize = 0;
    switch (indexType)
    {
    case IndexType::UInt16:
        vkIndexType = vk::IndexType::eUint16;
        indexSize = sizeof(uint16_t);
        break;
    case IndexType::UInt32:
        vkIndexType = vk::IndexType::eUint32;
        indexSize = sizeof(uint32_t);
        break;
    default:
        Log::Fatal("Unsupported index type.");
    }
    if (offset % indexSize != 0 || buffer.GetSize() - offset < indexSize)
    {
        Log::Fatal("Index buffer offset must be aligned and leave at least one complete index.");
    }
    m_CommandBuffer.bindIndexBuffer(vulkanBuffer.GetHandle(), offset, vkIndexType);
    m_BoundIndexCount = (buffer.GetSize() - offset) / indexSize;
}

void VulkanCommandBuffer::Draw(const uint32_t vertexCount, const uint32_t instanceCount)
{
    ValidateResources();
    m_CommandBuffer.draw(vertexCount, instanceCount, 0, 0);
}

void VulkanCommandBuffer::DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount)
{
    ValidateResources();
    if (m_BoundIndexCount == 0 || indexCount > m_BoundIndexCount)
    {
        Log::Fatal("Indexed draw requires a bound index buffer with enough indices.");
    }
    m_CommandBuffer.drawIndexed(indexCount, instanceCount, 0, 0, 0);
}

}
