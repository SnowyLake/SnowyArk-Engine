#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/RHI.h"

#include <vulkan/vulkan.hpp>

namespace Snowy::Ark
{
// VerifyFunc Concept
template<typename R, typename V>
concept IsVerifyFunc = requires(R result, V verifyFunc)
{
    verifyFunc(result);
};

// Forward Declare
class VulkanRHI;

// Vulkan Utils
class VulkanUtils
{
public:
    /*----------------------------------------------------------*/
    // Vulkan Result Process Function
    /*----------------------------------------------------------*/
    static void VerifyResult(vk::Result result, In<std::string> errorMsg, vk::Result targetResult = vk::Result::eSuccess)
    {
        if (result != targetResult)
        {
            throw std::runtime_error(static_cast<std::string>(errorMsg));
        }
    }
    template<typename T>
    static void VerifyResult(TIn(vk::ResultValue<T>) result, In<std::string> errorMsg, Out<T> output = nullptr, vk::Result targetResult = vk::Result::eSuccess)
    {
        if (result.result != targetResult)
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

    template<typename F> requires IsVerifyFunc<vk::Result, F>
    static void VerifyResult(vk::Result result, TIn(F) verifyFunc)
    {
        verifyFunc(result);
    }

    template<typename T, typename F> requires IsVerifyFunc<vk::ResultValue<T>, F>
    static void VerifyResult(TIn(vk::ResultValue<T>) result, TIn(F) verifyFunc)
    {
        verifyFunc(result);
    }


    /*----------------------------------------------------------*/
    // Vulkan Tool Functions
    /*----------------------------------------------------------*/
    static vk::Bool32 VKAPI_CALL ValidationLayerDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                              VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                              void* pUserData);

    static void CopyBuffer(RawHandle<VulkanRHI> vkContext, vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
};
}
