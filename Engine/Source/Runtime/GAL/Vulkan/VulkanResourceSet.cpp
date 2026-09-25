#include <Runtime/GAL/Vulkan/VulkanResourceSet.h>

#include <utility>

namespace SnowyArk
{
VulkanResourceSet::VulkanResourceSet(const VulkanPipelineState& pipeline, vk::raii::DescriptorPool pool, vk::raii::DescriptorSet set)
    : m_Pipeline(pipeline), m_Pool(std::move(pool)), m_Set(std::move(set))
{
}

const VulkanPipelineState& VulkanResourceSet::GetPipeline() const
{
    return m_Pipeline;
}

vk::DescriptorSet VulkanResourceSet::GetHandle() const
{
    return *m_Set;
}
}
