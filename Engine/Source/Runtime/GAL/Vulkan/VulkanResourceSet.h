#pragma once

#include <Runtime/GAL/ResourceSet.h>
#include <Runtime/GAL/Vulkan/VulkanInclude.h>

namespace SnowyArk
{
class VulkanPipelineState;

class VulkanResourceSet final : public ResourceSet
{
public:
    /// Owns a descriptor pool and its single set; borrows the creating pipeline until destruction.
    VulkanResourceSet(const VulkanPipelineState& pipeline, vk::raii::DescriptorPool pool, vk::raii::DescriptorSet set);

    /// Returns the pipeline whose layout defines these immutable bindings.
    const VulkanPipelineState& GetPipeline() const;

    /// Returns the native descriptor set for recording.
    vk::DescriptorSet GetHandle() const;

private:
    const VulkanPipelineState& m_Pipeline;
    vk::raii::DescriptorPool m_Pool;
    vk::raii::DescriptorSet m_Set;
};
}
