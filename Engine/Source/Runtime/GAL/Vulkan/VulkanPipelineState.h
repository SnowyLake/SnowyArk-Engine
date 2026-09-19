#pragma once

#include <Runtime/GAL/PipelineState.h>
#include <Runtime/GAL/Vulkan/VulkanInclude.h>

namespace SnowyArk
{

class VulkanPipelineState final : public PipelineState
{
public:
    /// Takes ownership of a Vulkan pipeline and its layout.
    VulkanPipelineState(vk::raii::PipelineLayout layout, vk::raii::Pipeline pipeline);

    /// Returns the native pipeline handle.
    vk::Pipeline GetPipeline() const;

private:
    vk::raii::PipelineLayout m_Layout;
    vk::raii::Pipeline m_Pipeline;
};

}
