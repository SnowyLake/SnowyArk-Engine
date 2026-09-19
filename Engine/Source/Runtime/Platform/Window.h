#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace SnowyArk
{

struct WindowDesc
{
    uint32_t width = 1280;
    uint32_t height = 720;
    const char* title = "SnowyArk";
    bool resizable = true;
};

struct WindowNativeHandle
{
    void* hwnd = nullptr;
    void* hinstance = nullptr;
};

class Window
{
public:
    Window() = default;
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    ~Window();

    /// Creates a GLFW window. Returns false without calling glfwInit when width or height is 0 or greater than INT_MAX.
    [[nodiscard]] bool Create(const WindowDesc& desc);

    /// Pumps pending platform events without blocking.
    void PumpEvents();

    /// Blocks until a platform event is available, then processes queued events.
    void WaitEvents();

    /// Returns true when the user has requested that the window close.
    bool ShouldClose() const;

    /// Returns true when both framebuffer dimensions are non-zero.
    bool HasDrawableExtent() const;

    /// Returns the current framebuffer width in pixels.
    uint32_t GetWidth() const;

    /// Returns the current framebuffer height in pixels.
    uint32_t GetHeight() const;

    /// Returns true once after the framebuffer size changes, then clears the flag.
    bool ConsumeResized();

    /// Returns the native window handles required by graphics backends. Valid until Destroy.
    const WindowNativeHandle& GetNativeHandle() const;

    /// Returns Vulkan instance extension names required by the window surface. Valid until Destroy.
    const std::vector<const char*>& GetInstanceExtensions() const;

    /// Destroys the window and shuts down GLFW.
    void Destroy();

private:
    /// Updates the framebuffer size stored on the Window that owns `glfwWindow`. Exceptions are contained because GLFW invokes this through a C ABI.
    static void OnFramebufferSize(void* glfwWindow, int width, int height);

    void* m_GlfwWindow = nullptr;
    WindowNativeHandle m_NativeHandle {};
    std::vector<const char*> m_InstanceExtensions;
    std::string m_Title;
    uint32_t m_Width = 0;
    uint32_t m_Height = 0;
    bool m_Resized = false;
};

}
