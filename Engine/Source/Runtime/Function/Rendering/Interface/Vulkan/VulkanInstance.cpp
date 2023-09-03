#include "VulkanInstance.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanRHI.h"
#include <GLFW/glfw3.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Snowy::Ark
{
using Utils = VulkanUtils;

void VulkanInstance::Init(ObserverHandle<VulkanRHI> vkContext, vk::InstanceCreateInfo createInfo, vk::Optional<const vk::AllocationCallbacks> allocator)
{
    m_Owner = vkContext;
    
    vk::DynamicLoader loader;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    if (m_EnableValidationLayers && CheckValidationLayersSupport(m_ValidationLayers) == false)
    {
        LOG_ERROR("Vaildation layers requested, but not available!");
    }

    createInfo.setPEnabledExtensionNames(m_RequiredExtensions);
    if (m_EnableValidationLayers)
    {
        createInfo.setPEnabledLayerNames(m_ValidationLayers);
    } else
    {
        createInfo.setPEnabledLayerNames(nullptr);
    }

    Utils::VerifyResult(vk::createInstance(createInfo, nullptr), "Failed to Create Vk Instance!", &m_Native);

    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_Native);

    if (m_EnableValidationLayers)
    {
        SetupDebugCallback();
    }
    CreateSurface();
    FetchAllAdapters();
}

void VulkanInstance::Destroy()
{
    if (m_EnableValidationLayers)
    {
        m_Native.destroyDebugUtilsMessengerEXT(m_Callback);
    } 
    m_Native.destroySurfaceKHR(m_Surface);
    m_Native.destroy();
}

void VulkanInstance::PrepareExtensionsAndLayers(In<RHIConfig> config)
{
#ifdef NDEBUG
    m_EnableValidationLayers = false;
#else
    m_EnableValidationLayers = true & config.vkEnableValidationLayers;
#endif
    m_ValidationLayers = config.vkValidationLayers;

    uint32_t glfwExtensionCount = 0;
    const AnsiChar** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    m_RequiredExtensions = std::vector<const AnsiChar*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (m_EnableValidationLayers)
    {
        m_RequiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
}

void VulkanInstance::FetchAllAdapters()
{
    Utils::VerifyResult(m_Native.enumeratePhysicalDevices(),
                        [this](const auto& result) {
                            auto& [r, gpus] = result;
                            if (r != vk::Result::eSuccess || gpus.empty())
                            {
                                LOG_ERROR("Faild to find GPUs with Vulkan support!");
                            } else
                            {
                                auto gpuCount = gpus.size();
                                m_Adapters.resize(gpuCount);
                                for (size_t i = 0; i < gpuCount; i++)
                                {
                                    m_Adapters[i].SetOwner(this);
                                    m_Adapters[i].Native() = gpus[i];
                                    m_Adapters[i].QueryProperties();
                                    m_Adapters[i].QueryQueueFamilyIndices();
                                    m_Adapters[i].QuerySwapchainSupport();
                                }
                            }
                        });
}

void VulkanInstance::SetupDebugCallback()
{
    vk::DebugUtilsMessengerCreateInfoEXT createInfo = {
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        .pfnUserCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                            VkDebugUtilsMessageTypeFlagsEXT messageType,
                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                            void* pUserData) -> vk::Bool32
                            {
                                LOG_INFO(pCallbackData->pMessage);
                                return SA_RHI_FALSE;
                            },
        .pUserData = nullptr,
    };
    Utils::VerifyResult(m_Native.createDebugUtilsMessengerEXT(createInfo), "Failed to set up debug callback!", &m_Callback);
}

void VulkanInstance::CreateSurface()
{
    if (glfwCreateWindowSurface(m_Native, m_Owner->m_WindowHandle, nullptr, reinterpret_cast<decltype(m_Surface)::NativeType*>(&m_Surface)) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create vulkan surface!");
    }
}

bool VulkanInstance::CheckValidationLayersSupport(ArrayIn<const AnsiChar*> inValidationLayers)
{
    bool support = true;
    Utils::VerifyResult(vk::enumerateInstanceLayerProperties(),
                        [&](const auto& result) {
                            auto& [r, properties] = result;
                            if (r != vk::Result::eSuccess)
                            {
                                LOG_ERROR("Failed to enumerate instance layer properties!");
                            } else
                            {
                                for (const AnsiChar* layerName : inValidationLayers)
                                {
                                    bool layerFound = false;
                                    for (const auto& property : properties)
                                    {
                                        if (strcmp(layerName, property.layerName) == 0)
                                        {
                                            layerFound = true;
                                            break;
                                        }
                                    }
                                    if (!layerFound)
                                    {
                                        support = false;
                                    }
                                }
                            }
                        });
    return support;
}
}