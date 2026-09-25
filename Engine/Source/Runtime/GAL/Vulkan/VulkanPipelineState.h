#pragma once

#include <vector>

#include <Runtime/GAL/PipelineState.h>
#include <Runtime/GAL/Vulkan/VulkanInclude.h>

namespace SnowyArk
{

class VulkanPipelineState final : public PipelineState
{
public:
    /// Takes ownership of a Vulkan pipeline and its layout.
    VulkanPipelineState(vk::raii::DescriptorSetLayout resourceLayout, vk::raii::PipelineLayout layout, vk::raii::Pipeline pipeline, std::vector<UniformBinding> uniformBindings);

    /// Returns the native pipeline handle.
    vk::Pipeline GetPipeline() const;

    /// Returns the owning device for cross-device validation.
    vk::Device GetDevice() const;

    /// Returns the layout used for binding descriptor sets.
    vk::PipelineLayout GetLayout() const;

    /// Returns set 0's native layout, or null for a pipeline without uniforms.
    vk::DescriptorSetLayout GetResourceLayout() const;

    /// Returns the immutable layout copied at pipeline creation.
    std::span<const UniformBinding> GetUniformBindings() const;

private:
    vk::raii::DescriptorSetLayout m_ResourceLayout;
    vk::raii::PipelineLayout m_Layout;
    vk::raii::Pipeline m_Pipeline;
    std::vector<UniformBinding> m_UniformBindings;
};

}
