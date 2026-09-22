#pragma once

#include <memory>
#include <span>
#include <vector>

#include <Runtime/GAL/GraphicsDevice.h>
#include <Runtime/GAL/Vulkan/VulkanCommandBuffer.h>
#include <Runtime/GAL/Vulkan/VulkanInclude.h>

namespace SnowyArk
{

class VulkanDevice final : public GraphicsDevice
{
public:
    VulkanDevice() = default;

    /// Releases Vulkan objects. Cleanup waits that fail are logged and do not escape the destructor.
    ~VulkanDevice() noexcept override;

    /// Creates a Vulkan graphics device.
    [[nodiscard]] static std::unique_ptr<GraphicsDevice> Create();

    bool Initialize(const WindowNativeHandle& nativeHandle, std::span<const char* const> instanceExtensions) override;
    std::unique_ptr<SwapChain> CreateSwapChain(const Window& window) override;
    std::unique_ptr<PipelineState> CreateGraphicsPipeline(const GraphicsPipelineDesc& desc) override;
    std::unique_ptr<Buffer> CreateBuffer(const BufferDesc& desc) override;
    CommandBuffer* BeginFrame(SwapChain& swapChain) override;
    void EndFrame(SwapChain& swapChain) override;
    void WaitIdle() override;

    /// Releases Vulkan objects without throwing after a failed cleanup wait.
    void Shutdown() noexcept override;

private:
    /// Creates the Vulkan instance and optional debug messenger.
    void CreateInstance(std::span<const char* const> instanceExtensions);

    /// Creates a Win32 presentation surface from the native window handles.
    void CreateSurface(const WindowNativeHandle& nativeHandle);

    /// Selects a Vulkan 1.4 GPU that reports required device features and can present R8G8B8A8Srgb with SrgbNonlinear.
    void PickPhysicalDevice();

    /// Creates the logical device and graphics/present queues.
    void CreateLogicalDevice();

    /// Creates the per-frame command pool and the transient pool used for buffer uploads.
    void CreateCommandPool();

    /// Allocates per-frame command buffers, acquire semaphores, and in-flight fences.
    void CreateFrameResources();

    /// Recreates image-indexed render-finished semaphores when the swapchain image count changes.
    /// Builds the new set first, then waits for GPU idle and replaces the previous set so a failed create leaves the old semaphores in place.
    void EnsureRenderFinishedSemaphores(uint32_t imageCount);

    /// Forwards Vulkan debug-utils messages to the engine log. Catches log failures so exceptions do not cross the C ABI.
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT,
                                                          const vk::DebugUtilsMessengerCallbackDataEXT* callbackData, void*);

    /// Returns true when `name` appears in the instance or device layer list.
    static bool HasLayer(const std::vector<vk::LayerProperties>& layers, const char* name);

    /// Returns true when `name` appears in the instance or device extension list.
    static bool HasExtension(const std::vector<vk::ExtensionProperties>& extensions, const char* name);

    /// Builds the debug-messenger create info that points at DebugCallback.
    static vk::DebugUtilsMessengerCreateInfoEXT MakeDebugMessengerCreateInfo();

    struct AllocatedBuffer
    {
        vk::raii::DeviceMemory memory = nullptr;
        vk::raii::Buffer buffer = nullptr;
    };

    /// Allocates a buffer and binds one dedicated device-memory allocation.
    AllocatedBuffer CreateAllocatedBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

    /// Returns a memory type that satisfies `typeFilter` and every flag in `properties`.
    uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

    /// Copies `size` bytes, makes transfer writes visible to `dstStage`, and waits for the upload fence; failed waits drain the device before unwinding.
    void CopyBuffer(vk::Buffer source, vk::Buffer destination, vk::DeviceSize size, vk::PipelineStageFlags2 dstStage, vk::AccessFlags2 dstAccess);

    static constexpr uint32_t k_MaxFramesInFlight = 2;

#ifndef NDEBUG
    static constexpr bool k_EnableValidation = true;
#else
    static constexpr bool k_EnableValidation = false;
#endif

    vk::raii::Context m_Context;
    vk::raii::Instance m_Instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT m_DebugMessenger = nullptr;
    vk::raii::SurfaceKHR m_Surface = nullptr;
    vk::raii::PhysicalDevice m_PhysicalDevice = nullptr;
    vk::raii::Device m_Device = nullptr;
    vk::raii::Queue m_GraphicsQueue = nullptr;
    vk::raii::Queue m_PresentQueue = nullptr;
    vk::raii::CommandPool m_CommandPool = nullptr;
    vk::raii::CommandPool m_UploadCommandPool = nullptr;
    vk::raii::CommandBuffers m_VkCommandBuffers = nullptr;
    std::vector<std::unique_ptr<VulkanCommandBuffer>> m_CommandBuffers;
    std::vector<vk::raii::Semaphore> m_ImageAvailableSemaphores;
    std::vector<vk::raii::Semaphore> m_RenderFinishedSemaphores;
    std::vector<vk::raii::Fence> m_InFlightFences;
    uint32_t m_GraphicsQueueFamily = 0;
    uint32_t m_PresentQueueFamily = 0;
    uint32_t m_FrameIndex = 0;
    uint32_t m_ImageIndex = 0;
};

}
