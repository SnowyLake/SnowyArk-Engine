#pragma once
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanUtils.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanAdapter.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/Vulkan/VulkanDevice.h"

namespace Snowy::Ark
{
class VulkanInstance
{
public:
    using NativeType = vk::Instance;
    using OwnerType  = VulkanRHI;

    VulkanInstance() = default;
    ~VulkanInstance() = default;
    VulkanInstance(const VulkanInstance&) = default;
    VulkanInstance(VulkanInstance&&) = default;
    VulkanInstance& operator=(const VulkanInstance&) = default;
    VulkanInstance& operator=(VulkanInstance&&) = default;

    void Init(ObserverHandle<OwnerType> owner) noexcept;
    void Destroy() noexcept;

    auto& Native    () noexcept { return m_Native; }
    auto& Native    () const noexcept { return m_Native; }
    auto& operator* () noexcept { return m_Native; }
    auto& operator* () const noexcept { return m_Native; }
    auto* operator->() noexcept { return &m_Native; }
    auto* operator->() const noexcept { return &m_Native; }
    operator NativeType() const noexcept { return m_Native; }
    operator NativeType::NativeType() const noexcept { return m_Native; }
    ObserverHandle<OwnerType> Owner() const noexcept { return m_Owner; }
    ObserverHandle<VulkanRHI> Context() const noexcept { return m_Ctx; }

    ObserverHandle<GLFWwindow> GetWindowHandle() const noexcept { return m_WindowHandle; }
    void SetWindowHandle(ObserverHandle<GLFWwindow> handle) noexcept { m_WindowHandle = handle; }

    const vk::SurfaceKHR& Surface() const noexcept { return m_Surface; }
    ObserverHandle<VulkanAdapter> Adapter(uint32_t idx) noexcept { return &(m_Adapters[idx]); }
    uint32_t AdapterCount() const noexcept { return static_cast<uint32_t>(m_Adapters.size()); }

    bool& EnableValidationLayers() noexcept { return m_EnableValidationLayers; }
    std::vector<const AnsiChar*>& ValidationLayers() noexcept { return m_ValidationLayers; }
    std::vector<const AnsiChar*>& RequiredExtensions() noexcept { return m_RequiredExtensions; }
    std::vector<const AnsiChar*>& RequiredDeviceExtensions() noexcept { return m_RequiredDeviceExtensions; }
    
    uint32_t GetFrameCountInFlight() const noexcept { return m_FrameCountInFlight; }
    void SetFrameCountInFlight(uint32_t count) noexcept { m_FrameCountInFlight = count; }

    VulkanDevice CreateDevice() noexcept;

private:
    void CollectAdapters() noexcept;
    void SetupDebugCallback() noexcept;
    void CreateSurface() noexcept;

    bool CheckValidationLayersSupport(ArrayIn<const AnsiChar*> validationLayers) noexcept;

private:
    NativeType m_Native;
    ObserverHandle<OwnerType> m_Owner;
    ObserverHandle<VulkanRHI> m_Ctx;

    ObserverHandle<GLFWwindow> m_WindowHandle;

    vk::SurfaceKHR m_Surface;
    vk::DebugUtilsMessengerEXT m_Callback;
    std::vector<VulkanAdapter> m_Adapters;

    bool m_EnableValidationLayers;
    std::vector<const AnsiChar*> m_ValidationLayers;
    std::vector<const AnsiChar*> m_RequiredExtensions;
    std::vector<const AnsiChar*> m_RequiredDeviceExtensions;

    uint32_t m_FrameCountInFlight;
};
}
