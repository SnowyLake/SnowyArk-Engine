#pragma once

#include <memory>
#include <span>

#include <Runtime/GAL/GraphicsTypes.h>

namespace SnowyArk
{

class CommandBuffer;
class PipelineState;
class SwapChain;
class Window;
struct GraphicsPipelineDesc;
struct WindowNativeHandle;

class GraphicsDevice
{
public:
    GraphicsDevice(const GraphicsDevice&) = delete;
    GraphicsDevice& operator=(const GraphicsDevice&) = delete;
    virtual ~GraphicsDevice() = default;

    /// Creates a graphics device for the requested backend. Returns null when the backend is unsupported.
    [[nodiscard]] static std::unique_ptr<GraphicsDevice> Create(GraphicsBackend backend);

    /// Creates the instance, debug messenger, surface, and logical device. Returns false on recoverable failure; unrecoverable setup errors throw.
    [[nodiscard]] virtual bool Initialize(const WindowNativeHandle& nativeHandle, std::span<const char* const> instanceExtensions) = 0;

    /// Creates a swapchain for the given window. `window` must remain alive for the lifetime of the returned swapchain.
    virtual std::unique_ptr<SwapChain> CreateSwapChain(const Window& window) = 0;

    /// Creates a graphics pipeline from the supplied description. `desc.shaderSpirv` is borrowed only for this call.
    virtual std::unique_ptr<PipelineState> CreateGraphicsPipeline(const GraphicsPipelineDesc& desc) = 0;

    /// Acquires the next swapchain image and begins recording.
    /// Returns a borrowed command buffer valid until EndFrame, the next BeginFrame on this device, swapchain recreation, or Shutdown. Returns null when this frame must be skipped.
    /// May wait for the in-flight fence of the current frame slot.
    virtual CommandBuffer* BeginFrame(SwapChain& swapChain) = 0;

    /// Submits the recorded command buffer and presents the image.
    virtual void EndFrame(SwapChain& swapChain) = 0;

    /// Waits until the GPU is idle. Throws on failure, including device lost.
    virtual void WaitIdle() = 0;

    /// Releases device-owned GPU objects without throwing. A failed cleanup wait is logged and remaining objects are still released.
    virtual void Shutdown() noexcept = 0;

protected:
    GraphicsDevice() = default;
};

}
