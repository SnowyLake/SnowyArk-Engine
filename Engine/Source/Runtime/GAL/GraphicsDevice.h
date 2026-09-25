#pragma once

#include <memory>
#include <span>

#include <Runtime/GAL/GraphicsTypes.h>

namespace SnowyArk
{

class Buffer;
class CommandBuffer;
class PipelineState;
class ResourceSet;
class SwapChain;
class Window;
struct BufferDesc;
struct GraphicsPipelineDesc;
struct WindowNativeHandle;
struct UniformBufferBinding;

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

    /// Creates a graphics pipeline. Shader code and all vertex/uniform layout spans are borrowed only for this call.
    /// Invalid layouts or winding, missing shader code or entry names, and backend failures throw; shader/layout agreement remains the caller's responsibility.
    virtual std::unique_ptr<PipelineState> CreateGraphicsPipeline(const GraphicsPipelineDesc& desc) = 0;

    /// Creates a device-local Vertex/Index buffer with a synchronous upload, or a host-visible coherent Uniform buffer.
    /// Synchronous initialization API; may block on earlier GPU work. `desc.initialData` is borrowed only for this call.
    /// Empty Vertex/Index data returns null. Uniform size must be nonzero; optional initial data must fit and unwritten bytes start at zero.
    /// The buffer must be destroyed before this device, after any GPU use has finished.
    /// Invalid usage and backend failures throw. An upload failure requires application cleanup before any further rendering.
    virtual std::unique_ptr<Buffer> CreateBuffer(const BufferDesc& desc) = 0;

    /// Creates immutable set 0 bindings matching every uniform binding of `pipeline` exactly once.
    /// Requires same-device Uniform buffers, nonzero in-range sizes and device-aligned offsets. Invalid descriptions throw.
    /// Pipeline and buffers are borrowed until set destruction; destroy the set after GPU completion and before those objects/device.
    virtual std::unique_ptr<ResourceSet> CreateResourceSet(const PipelineState& pipeline, std::span<const UniformBufferBinding> bindings) = 0;

    /// Returns the fixed number of in-flight slots for allocating independently writable per-frame resources.
    virtual uint32_t GetFrameCount() const = 0;

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
