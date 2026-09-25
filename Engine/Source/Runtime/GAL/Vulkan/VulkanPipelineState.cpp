#include <Runtime/GAL/Vulkan/VulkanPipelineState.h>

#include <utility>

namespace SnowyArk
{

VulkanPipelineState::VulkanPipelineState(vk::raii::DescriptorSetLayout resourceLayout, vk::raii::PipelineLayout layout, vk::raii::Pipeline pipeline, std::vector<UniformBinding> uniformBindings)
    : m_ResourceLayout(std::move(resourceLayout)), m_Layout(std::move(layout)), m_Pipeline(std::move(pipeline)), m_UniformBindings(std::move(uniformBindings))
{
}

vk::Pipeline VulkanPipelineState::GetPipeline() const
{
    return *m_Pipeline;
}

vk::Device VulkanPipelineState::GetDevice() const
{
    return m_Pipeline.getDevice();
}

vk::PipelineLayout VulkanPipelineState::GetLayout() const
{
    return *m_Layout;
}

vk::DescriptorSetLayout VulkanPipelineState::GetResourceLayout() const
{
    return *m_ResourceLayout;
}

std::span<const UniformBinding> VulkanPipelineState::GetUniformBindings() const
{
    return m_UniformBindings;
}

}
