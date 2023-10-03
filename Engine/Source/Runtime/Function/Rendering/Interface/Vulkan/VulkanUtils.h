#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Core/Log/Logger.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/RHI.h"

#include <vulkan/vulkan.hpp>

#include <type_traits>

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

class VulkanUtils
{
public:
    static constexpr inline std::array CommonDepthFormats = { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint };

public:
    // Vulkan Number Type
    using NumType = uint32_t;
    template<typename T>
        requires std::is_integral_v<T> || std::is_enum_v<T>
    static NumType CastNumType(T num) 
    { 
        return static_cast<NumType>(num);
    }
    #define SA_VK_NUM(x) VulkanUtils::CastNumType(x)


    static bool HasStencilComponent(vk::Format format);

    /*----------------------------------------------------------*/
    // Vulkan Result Process Function
    /*----------------------------------------------------------*/
    static void VerifyResult(vk::Result result, SStringIn errorMsg, vk::Result targetResult = vk::Result::eSuccess)
    {
        if (result != targetResult)
        {
            SA_LOG_ERROR_SSTR(errorMsg);
        }
    }

    template<typename T>
    static void VerifyResult(In<vk::ResultValue<T>> result, SStringIn errorMsg, Out<T> output = nullptr, vk::Result targetResult = vk::Result::eSuccess)
    {
        if (result.result != targetResult)
        {
            SA_LOG_ERROR_SSTR(errorMsg);
        } else
        {
            if (output)
            {
                *output = result.value;
            }
        }
    }

    template<typename F>
        requires IsVerifyFunc<vk::Result, F>
    static void VerifyResult(vk::Result result, In<F> verifyFunc)
    {
        verifyFunc(result);
    }

    template<typename T, typename F>
        requires IsVerifyFunc<vk::ResultValue<T>, F>
    static void VerifyResult(In<vk::ResultValue<T>> result, In<F> verifyFunc)
    {
        verifyFunc(result);
    }
};
}
