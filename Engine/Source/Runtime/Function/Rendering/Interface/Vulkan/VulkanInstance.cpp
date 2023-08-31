#include "VulkanInstance.h"
#include <GLFW/glfw3.h>
namespace Snowy::Ark
{
using Utils = VulkanUtils;

void VulkanInstance::Init(vk::InstanceCreateInfo createInfo, vk::Optional<const vk::AllocationCallbacks> allocator)
{
    if (enableValidationLayers && Utils::CheckValidationLayersSupport(validationLayers) == false)
    {
        LOG_ERROR("Vaildation layers requested, but not available!");
    }
    Utils::VerifyResult(vk::createInstance(createInfo, nullptr), "Failed to Create Vk Instance!", &m_Instance);
    FetchAllAdapters();
}
void VulkanInstance::Destroy()
{

}

void VulkanInstance::PrepareExtensionsAndLayers(In<RHIConfig> config)
{
#ifdef NDEBUG
    enableValidationLayers = false;
#else
    enableValidationLayers = true & config.vkEnableValidationLayers;
#endif
    validationLayers = config.vkValidationLayers;

    uint32_t glfwExtensionCount = 0;
    RawHandle<RawHandle<const AnsiChar>> glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    requiredExtensions = std::vector<RawHandle<const AnsiChar>>(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (enableValidationLayers)
    {
        requiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
}

void VulkanInstance::FetchAllAdapters()
{
    Utils::VerifyResult(m_Instance.enumeratePhysicalDevices(),
                        [this](const auto& result) {
                            auto& [r, physicalDevices] = result;
                            if (r != vk::Result::eSuccess || physicalDevices.empty())
                            {
                                LOG_ERROR("Faild to find GPUs with Vulkan support!");
                            } else
                            {
                                auto count = physicalDevices.size();
                                m_Adapters.resize(count);
                                for (size_t i = 0; i < count; i++)
                                {
                                    m_Adapters[i].physicalDevice = physicalDevices[i];
                                    m_Adapters[i].QueryProperties();
                                    m_Adapters[i].QueryQueueFamilyIndices();
                                    m_Adapters[i].QuerySwapChainSupport();
                                    m_Adapters[i].instance = this;
                                }
                            }
                        });
}
}