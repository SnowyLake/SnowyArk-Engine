#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include <vulkan/vulkan.hpp>

namespace Snowy::Ark
{
template<typename R, typename Processor>
concept IsProcessor = requires(R r, Processor processor)
{
    processor(r);
};

class VulkanUtils
{
public:
    static void ExecResult(vk::Result result, In<std::string> errorMsg, vk::Result compare = vk::Result::eSuccess)
    {
        if (result != compare)
        {
            throw std::runtime_error(static_cast<std::string>(errorMsg));
        }
    }

    template<typename T>
    static void ExecResult(const vk::ResultValue<T>& result, In<std::string> errorMsg, Out<T> output = nullptr, vk::Result compare = vk::Result::eSuccess)
    {
        if (result.result != compare)
        {
            throw std::runtime_error(static_cast<std::string>(errorMsg));
        } else
        {
            if (output)
            {
                *output = result.value;
            }
        }
    }

    template<typename P> requires IsProcessor<vk::Result, P>
    static void ExecResult(vk::Result result, const P& processor)
    {
        processor(result);
    }

    template<typename T, typename P> requires IsProcessor<vk::ResultValue<T>, P>
    static void ExecResult(const vk::ResultValue<T>& result, const P& processor)
    {
        processor(result);
    }

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL ValidationLayerDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                         VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                         const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                                         void* pUserData)
    {
        std::cerr << std::format("Validation layer: {}\n", pCallbackData->pMessage);
        return VK_FALSE;
    }
};
}
