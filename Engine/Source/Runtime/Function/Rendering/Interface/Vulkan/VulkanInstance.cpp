#include "VulkanInstance.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanRHI.h"
#include <GLFW/glfw3.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Snowy::Ark
{
using Utils = VulkanUtils;

void VulkanInstance::Init(ObserverHandle<OwnerType> owner) noexcept
{
    m_Owner = owner;
    m_Ctx = owner;
    
    vk::DynamicLoader loader;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    vk::ApplicationInfo appInfo = {
        .pApplicationName = "VulkanRHI",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "SnowyArk",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_1,
    };

    vk::InstanceCreateInfo createInfo = {
        .pApplicationInfo = &appInfo,
    };

    if (m_EnableValidationLayers && CheckValidationLayersSupport(m_ValidationLayers) == false)
    {
        SA_LOG_ERROR(STEXT("Vaildation layers requested, but not available!"));
    }

    createInfo.setPEnabledExtensionNames(m_RequiredExtensions);
    if (m_EnableValidationLayers)
    {
        createInfo.setPEnabledLayerNames(m_ValidationLayers);
    } else
    {
        createInfo.setPEnabledLayerNames(nullptr);
    }

    Utils::VerifyResult(vk::createInstance(createInfo, nullptr), STEXT("Failed to Create Vk Instance!"), &m_Native);

    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_Native);

    if (m_EnableValidationLayers)
    {
        SetupDebugCallback();
    }
    CreateSurface();
    CollectAdapters();
}

void VulkanInstance::Destroy() noexcept
{
    if (m_EnableValidationLayers)
    {
        m_Native.destroyDebugUtilsMessengerEXT(m_Callback);
    } 
    m_Native.destroySurfaceKHR(m_Surface);
    m_Native.destroy();
}

VulkanDevice VulkanInstance::CreateDevice() noexcept
{
    VulkanDevice device;
    device.RequiredExtensions().append_range(m_RequiredDeviceExtensions);
    device.Init(this);
    SA_LOG_INFO(STEXT("Vulkan Device Initialized."));
    return device;
}

void VulkanInstance::CollectAdapters() noexcept
{
    Utils::VerifyResult(m_Native.enumeratePhysicalDevices(),
                        [this](const auto& result) {
                            auto& [r, gpus] = result;
                            if (r != vk::Result::eSuccess || gpus.empty())
                            {
                                SA_LOG_ERROR(STEXT("Faild to find GPUs with Vulkan support!"));
                            } else
                            {
                                auto gpuCount = gpus.size();
                                m_Adapters.resize(gpuCount);
                                for (size_t i = 0; i < gpuCount; i++)
                                {
                                    m_Adapters[i].Init(this, gpus[i]);
                                    m_Adapters[i].QueryProperties();
                                    m_Adapters[i].QueryQueueFamilyIndices();
                                }
                            }
                        });
}

void VulkanInstance::SetupDebugCallback() noexcept
{
    vk::DebugUtilsMessengerCreateInfoEXT createInfo = {
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        .pfnUserCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                            VkDebugUtilsMessageTypeFlagsEXT messageType,
                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                            void* pUserData) -> vk::Bool32
                            {
                                SA_LOG_INFO(ANSI_TO_SSTR(pCallbackData->pMessage));
                                return SA_RHI_FALSE;
                            },
        .pUserData = nullptr,
    };
    Utils::VerifyResult(m_Native.createDebugUtilsMessengerEXT(createInfo), STEXT("Failed to set up debug callback!"), &m_Callback);
}

void VulkanInstance::CreateSurface() noexcept
{
    if (glfwCreateWindowSurface(m_Native, m_WindowHandle, nullptr, reinterpret_cast<decltype(m_Surface)::NativeType*>(&m_Surface)) != VK_SUCCESS)
    {
        SA_LOG_ERROR(STEXT("Failed to create vulkan surface!"));
    }
}

bool VulkanInstance::CheckValidationLayersSupport(ArrayIn<const AnsiChar*> validationLayers) noexcept
{
    bool support = true;
    Utils::VerifyResult(vk::enumerateInstanceLayerProperties(),
                        [&](const auto& result) {
                            auto& [r, properties] = result;
                            if (r != vk::Result::eSuccess)
                            {
                                SA_LOG_ERROR(STEXT("Failed to enumerate instance layer properties!"));
                            } else
                            {
                                for (const AnsiChar* layerName : validationLayers)
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
