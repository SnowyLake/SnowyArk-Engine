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

    static void ResultProcessing(vk::Result result, In<std::string> errorMsg, vk::Result compare = vk::Result::eSuccess)
    {
        if (result != compare)
        {
            throw std::runtime_error(static_cast<std::string>(errorMsg));
        }
    }

    template<typename T>
    static void ResultProcessing(const vk::ResultValue<T>& result, In<std::string> errorMsg, Out<T> target = nullptr, vk::Result compare = vk::Result::eSuccess)
    {
        if (result.result != compare)
        {
            throw std::runtime_error(static_cast<std::string>(errorMsg));
        } else
        {
            if (target)
            {
                *target = result.value;
            }
        }
    }

    template<typename P> requires IsProcessor<vk::Result, P>
    static void ResultProcessing(vk::Result result, const P& processor)
    {
        processor(result);
    }

    template<typename T, typename P> requires IsProcessor<vk::ResultValue<T>, P>
    static void ResultProcessing(const vk::ResultValue<T>& result, const P& processor)
    {
        processor(result);
    }
};
}
