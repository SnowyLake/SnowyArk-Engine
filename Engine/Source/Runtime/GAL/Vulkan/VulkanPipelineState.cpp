#include <Runtime/GAL/Vulkan/VulkanPipelineState.h>

#include <utility>

namespace SnowyArk
{

VulkanPipelineState::VulkanPipelineState(vk::raii::PipelineLayout layout, vk::raii::Pipeline pipeline)
    : m_Layout(std::move(layout)), m_Pipeline(std::move(pipeline))
{
}

vk::Pipeline VulkanPipelineState::GetPipeline() const
{
    return *m_Pipeline;
}

}
