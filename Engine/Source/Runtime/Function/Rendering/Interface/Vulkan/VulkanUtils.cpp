#include "VulkanUtils.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanRHI.h"
namespace Snowy::Ark
{
vk::Bool32 VKAPI_CALL VkUtils::ValidationLayerDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
                                                            VkDebugUtilsMessageTypeFlagsEXT messageType, 
                                                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
                                                            void* pUserData)

{
    std::cerr << std::format("Validation layer: {}\n", pCallbackData->pMessage);
    return RHI_FALSE;
}

void VkUtils::CopyBuffer(Handle<VulkanRHI> vkHandle, vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
    auto& device = vkHandle->Device();
    auto& cmdPool = vkHandle->CommandPool();
    auto& submitQueue = vkHandle->GraphicsQueue();

    vk::CommandBufferAllocateInfo allocInfo = {
        .commandPool = cmdPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };

    vk::CommandBuffer cmd;
    VerifyResult(device.allocateCommandBuffers(allocInfo),
                 [&](const auto& result) {
                     if (result.result != vk::Result::eSuccess)
                     {
                         throw std::runtime_error("Failed to allocate copybuffer command!");
                     } else
                     {
                         cmd = result.value[0];
                     }
                 });

    vk::CommandBufferBeginInfo beginInfo = {
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    };
    VerifyResult(cmd.begin(beginInfo),
                 [&](auto result) {
                     if (result != vk::Result::eSuccess)
                     {
                         throw std::runtime_error("Failed to begin recording copybuffer command!");
                     } else
                     {
                         vk::BufferCopy copyRegion = {
                             .srcOffset = 0,
                             .dstOffset = 0,
                             .size = size,
                         };
                         cmd.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
                         VerifyResult(cmd.end(), "Failed to end recording copybuffer command!");
                     }
                 });

    vk::SubmitInfo submitInfo = {
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };
    VerifyResult(submitQueue.submit(submitInfo), "Failed to submit copybuffer command!");
    auto _ = submitQueue.waitIdle();
    device.freeCommandBuffers(cmdPool, cmd);
}
}