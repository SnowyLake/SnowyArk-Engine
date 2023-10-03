#include "VulkanTexture.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanRHI.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanDevice.h"
namespace Snowy::Ark
{
using Utils = VulkanUtils;

void VulkanTexture::Init(ObserverHandle<OwnerType> owner, In<TextureData> data, In<VulkanTextureParams> params)
{
    m_Owner = owner;
    m_Ctx = owner->Context();

    vk::ImageCreateInfo info = {
        .flags = {},
        .imageType = params.type,
        .format = params.format,
        .extent = vk::Extent3D {
            .width = SA_VK_NUM(data.width),
            .height = SA_VK_NUM(data.height),
            .depth = 1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = params.tiling,
        .usage = params.usage,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined,
    };

    Utils::VerifyResult(m_Owner->Native().createImage(info), STEXT("Failed to create image!"), &m_Native);

    vk::MemoryRequirements memRequirements = m_Owner->Native().getImageMemoryRequirements(m_Native);
    vk::MemoryAllocateInfo allocInfo = {
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = m_Owner->FindMemoryType(memRequirements.memoryTypeBits, params.memoryProps),
    };

    Utils::VerifyResult(m_Owner->Native().allocateMemory(allocInfo), STEXT("Failed to allocate memory!"), &m_Memory);
    Utils::VerifyResult(m_Owner->Native().bindImageMemory(m_Native, m_Memory, 0), STEXT("Failed to bind texture memory!"));

    // View
    vk::ImageViewCreateInfo viewInfo = {
        .image = m_Native,
        .viewType = params.viewType,
        .format = params.format,
        .subresourceRange = {
            .aspectMask = params.aspectMask,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    Utils::VerifyResult(m_Owner->Native().createImageView(viewInfo), STEXT("Failed to create texture image view!"), &m_View);

    // Sample
    vk::SamplerCreateInfo sampleInfo = {
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .mipLodBias = 0.0f,
        .anisotropyEnable = SA_RHI_TRUE,
        .maxAnisotropy = 16,
        .compareEnable = SA_RHI_FALSE,
        .compareOp = vk::CompareOp::eAlways,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = vk::BorderColor::eIntOpaqueBlack,
        .unnormalizedCoordinates = SA_RHI_FALSE,
    };
    Utils::VerifyResult(m_Owner->Native().createSampler(sampleInfo), STEXT("Failed to create texture sampler!"), &m_Sampler);
}

void VulkanTexture::Destroy()
{
    m_Owner->Native().destroySampler(m_Sampler);
    m_Owner->Native().destroyImageView(m_View);
    m_Owner->Native().destroyImage(m_Native);
    m_Owner->Native().freeMemory(m_Memory);
}

void VulkanTexture::TransitionLayout(vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
    auto cmd = m_Ctx->BeginSingleTimeCommandBuffer();

    vk::ImageMemoryBarrier barrier = {
        .srcAccessMask = {},
        .dstAccessMask = {},
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = m_Native,
        .subresourceRange = vk::ImageSubresourceRange {
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
    {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        if (Utils::HasStencilComponent(format))
        {
            barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
        }
    } else
    {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    }


    vk::PipelineStageFlags srcStages, dstStages;
    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        srcStages = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStages = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        srcStages = vk::PipelineStageFlagBits::eTransfer;
        dstStages = vk::PipelineStageFlagBits::eFragmentShader;
    } else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
    {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        srcStages = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStages = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    }
    else
    {
        SA_LOG_ERROR("Unsupported layout transition!");
        return;
    }

    cmd.pipelineBarrier(srcStages, dstStages, {}, nullptr, nullptr, barrier);

    m_Ctx->EndSingleTimeCommandBuffer(cmd);
}
}
