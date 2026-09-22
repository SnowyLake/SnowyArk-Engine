#pragma once

#include <memory>

#include <Runtime/GAL/GraphicsDevice.h>
#include <Runtime/GAL/SwapChain.h>
#include <Runtime/RenderPipeline/RenderPipeline.h>
#include <Runtime/Shader/ShaderLibrary.h>

namespace SnowyArk
{

class Window;

class Application
{
public:
    Application();
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    /// Releases runtime resources. Cleanup waits that fail are logged and do not escape the destructor.
    ~Application() noexcept;

    /// Creates the device, swapchain, shaders, and indexed rectangle pipeline. `window` must remain alive until Shutdown or destruction.
    /// Returns false without changing existing objects if a window, device, or swapchain is already held. Other failures call Shutdown; unrecoverable GPU errors throw.
    [[nodiscard]] bool Initialize(Window& window);

    /// Advances one runtime frame. Does not wait for window events.
    /// Skips rendering when the window is not drawable or BeginFrame returns null. May wait on an in-flight GPU fence or block in acquire.
    void Tick();

    /// Releases runtime GPU and subsystem resources without throwing. A failed GPU wait is logged and remaining objects are still released.
    void Shutdown() noexcept;

private:
    Window* m_Window = nullptr;
    std::unique_ptr<GraphicsDevice> m_Device;
    std::unique_ptr<SwapChain> m_SwapChain;
    ShaderLibrary m_ShaderLibrary;
    RenderPipeline m_RenderPipeline;
};

}
