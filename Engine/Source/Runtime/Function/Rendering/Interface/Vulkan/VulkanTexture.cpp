#include "VulkanTexture.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanRHI.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanDevice.h"
namespace Snowy::Ark
{
using Utils = VulkanUtils;

void VulkanTexture::Init(ObserverHandle<OwnerType> owner, ObserverHandle<TextureData> data, ObserverHandle<TextureParams> params)
{
    m_Owner = owner;
    m_Ctx = owner->Context();

    vk::ImageCreateInfo info = {
        .flags = {},
        .imageType = vk::ImageType::e2D,
        .format = vk::Format::eR8G8B8A8Unorm,
        .extent = vk::Extent3D {
            .width = Utils::CastNumType(data->width),
            .height = Utils::CastNumType(data->height),
            .depth = 1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined,
    };

    Utils::VerifyResult(m_Owner->Native().createImage(info), STEXT("Failed to create image!"), &m_Native);

    vk::MemoryRequirements memRequirements = m_Owner->Native().getImageMemoryRequirements(m_Native);
    vk::MemoryAllocateInfo allocInfo = {
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = m_Owner->FindMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal),
    };

    Utils::VerifyResult(m_Owner->Native().allocateMemory(allocInfo), STEXT("Failed to allocate memory!"), &m_Memory);
    Utils::VerifyResult(m_Owner->Native().bindImageMemory(m_Native, m_Memory, 0), STEXT("Failed to bind texture memory!"));

    // View
    vk::ImageViewCreateInfo viewInfo = {
        .image = m_Native,
        .viewType = vk::ImageViewType::e2D,
        .format = vk::Format::eR8G8B8A8Unorm,
        .subresourceRange = vk::ImageSubresourceRange {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
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
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

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
    } else
    {
        SA_LOG_ERROR("Unsupported layout transition!");
        return;
    }

    cmd.pipelineBarrier(srcStages, dstStages, {}, nullptr, nullptr, barrier);

    m_Ctx->EndSingleTimeCommandBuffer(cmd);
}
}
