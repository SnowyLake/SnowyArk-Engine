#pragma once
#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/RHI.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanUtils.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanAdapter.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanInstance.h"

#include <vulkan/vulkan.hpp>

namespace Snowy::Ark
{
class VulkanDevice
{
public:
    void Init(ObserverHandle<VulkanAdapter> adapter);
    void Destroy();

    void PrepareExtensionsAndLayers(In<RHIConfig> config);

public:
    std::vector<const AnsiChar*> validationLayers;
    std::vector<const AnsiChar*> requiredExtensions;
    
private:
    vk::Device m_Device;
    RawHandle<VulkanAdapter> m_Adapter;
};
}
