#include "VulkanDevice.h"
namespace Snowy::Ark
{
void VulkanDevice::Init(ObserverHandle<VulkanAdapter> adapter)
{

}
void VulkanDevice::Destroy()
{}
void VulkanDevice::PrepareExtensionsAndLayers(In<RHIConfig> config)
{
    validationLayers = config.vkValidationLayers;
    requiredExtensions = config.vkDeviceExtensions;
}
}