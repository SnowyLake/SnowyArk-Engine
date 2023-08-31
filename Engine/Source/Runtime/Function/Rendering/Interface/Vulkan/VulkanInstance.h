#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/RHI.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanUtils.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanAdapter.h"

#include <vulkan/vulkan.hpp>

namespace Snowy::Ark
{
class VulkanInstance
{
public:
    void Init(vk::InstanceCreateInfo createInfo, vk::Optional<const vk::AllocationCallbacks> allocator = nullptr);
    void Destroy();

    auto& GetNative (this auto&& self) noexcept { return self.m_Instance; }
    auto& operator* (this auto&& self) noexcept { return self.m_Instance; }
    auto* operator->(this auto&& self) noexcept { return &(self.m_Instance); }
    operator vk::Instance() const noexcept { return m_Instance; }
    operator VkInstance()   const noexcept { return m_Instance; }

    auto& GetAdapter(this auto&& self, uint32_t idx) noexcept { return self.m_Adapters[idx]; }
    uint32_t GetAdapterCount() const noexcept { return static_cast<uint32_t>(m_Adapters.size()); }

    void PrepareExtensionsAndLayers(In<RHIConfig> config);

private:
    void FetchAllAdapters();

public:
    bool enableValidationLayers;
    std::vector<const AnsiChar*> validationLayers;
    std::vector<const AnsiChar*> requiredExtensions;

private:
    vk::Instance m_Instance;
    std::vector<VulkanAdapter> m_Adapters;
};
}