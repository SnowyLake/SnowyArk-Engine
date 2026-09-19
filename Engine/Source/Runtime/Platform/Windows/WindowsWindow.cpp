#include <Runtime/Platform/Windows/WindowsWindow.h>

#include <cstdint>
#include <limits>
#include <utility>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <GLFW/glfw3.h>
// GLFW_EXPOSE_NATIVE_WIN32 must be defined before glfw3native.h. NOMINMAX must be defined before that header includes windows.h.
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <Runtime/Core/Log.h>

namespace SnowyArk
{

Window::~Window()
{
    Destroy();
}

bool Window::Create(const WindowDesc& desc)
{
    if (m_GlfwWindow != nullptr)
    {
        Log::Error("Window has already been created.");
        return false;
    }

    if (desc.width == 0 || desc.height == 0)
    {
        Log::Error("Window size must be non-zero ({}x{}).", desc.width, desc.height);
        return false;
    }
    if (desc.width > static_cast<uint32_t>((std::numeric_limits<int>::max)()) || desc.height > static_cast<uint32_t>((std::numeric_limits<int>::max)()))
    {
        Log::Error("Window size is too large ({}x{}).", desc.width, desc.height);
        return false;
    }

    if (glfwInit() != GLFW_TRUE)
    {
        glfwTerminate();
        Log::Error("Failed to initialize GLFW.");
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, desc.resizable ? GLFW_TRUE : GLFW_FALSE);

    const char* const title = desc.title != nullptr ? desc.title : "SnowyArk";
    GLFWwindow* const glfwWindow = glfwCreateWindow(static_cast<int>(desc.width), static_cast<int>(desc.height), title, nullptr, nullptr);
    if (glfwWindow == nullptr)
    {
        glfwTerminate();
        Log::Error("Failed to create GLFW window.");
        return false;
    }

    m_GlfwWindow = glfwWindow;
    m_Title = title;
    glfwSetWindowUserPointer(glfwWindow, this);
    glfwSetFramebufferSizeCallback(glfwWindow, [](GLFWwindow* const window, const int width, const int height) { OnFramebufferSize(window, width, height); });

    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(glfwWindow, &framebufferWidth, &framebufferHeight);
    m_Width = framebufferWidth > 0 ? static_cast<uint32_t>(framebufferWidth) : 0;
    m_Height = framebufferHeight > 0 ? static_cast<uint32_t>(framebufferHeight) : 0;

    m_NativeHandle.hwnd = glfwGetWin32Window(glfwWindow);
    m_NativeHandle.hinstance = GetModuleHandleW(nullptr);

    uint32_t extensionCount = 0;
    const char** const extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
    if (extensions == nullptr || extensionCount == 0)
    {
        Destroy();
        Log::Error("GLFW did not report any Vulkan instance extensions.");
        return false;
    }
    m_InstanceExtensions.assign(extensions, extensions + extensionCount);

    Log::Info("Created window '{}' ({}x{}).", m_Title, m_Width, m_Height);
    return true;
}

void Window::PumpEvents()
{
    glfwPollEvents();
}

void Window::WaitEvents()
{
    if (m_GlfwWindow == nullptr)
    {
        return;
    }
    glfwWaitEvents();
}

bool Window::ShouldClose() const
{
    return m_GlfwWindow != nullptr && glfwWindowShouldClose(static_cast<GLFWwindow*>(m_GlfwWindow)) == GLFW_TRUE;
}

bool Window::HasDrawableExtent() const
{
    return m_Width > 0 && m_Height > 0;
}

uint32_t Window::GetWidth() const
{
    return m_Width;
}

uint32_t Window::GetHeight() const
{
    return m_Height;
}

bool Window::ConsumeResized()
{
    return std::exchange(m_Resized, false);
}

const WindowNativeHandle& Window::GetNativeHandle() const
{
    return m_NativeHandle;
}

const std::vector<const char*>& Window::GetInstanceExtensions() const
{
    return m_InstanceExtensions;
}

void Window::Destroy()
{
    if (m_GlfwWindow != nullptr)
    {
        glfwDestroyWindow(static_cast<GLFWwindow*>(m_GlfwWindow));
        m_GlfwWindow = nullptr;
        glfwTerminate();
    }
    m_NativeHandle = {};
    m_InstanceExtensions.clear();
    m_Width = 0;
    m_Height = 0;
}

void Window::OnFramebufferSize(void* const glfwWindow, const int width, const int height)
{
    try
    {
        auto* const window = static_cast<Window*>(glfwGetWindowUserPointer(static_cast<GLFWwindow*>(glfwWindow)));
        if (window == nullptr)
        {
            return;
        }

        window->m_Width = width > 0 ? static_cast<uint32_t>(width) : 0;
        window->m_Height = height > 0 ? static_cast<uint32_t>(height) : 0;
        window->m_Resized = true;
    }
    catch (...)
    {
    }
}

}
