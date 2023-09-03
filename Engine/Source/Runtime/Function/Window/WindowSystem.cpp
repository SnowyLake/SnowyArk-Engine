#include "WindowSystem.h"
#include "Engine/Source/Runtime/Core/Log/Logger.h"
namespace Snowy::Ark
{
void WindowSystem::Init(Ref<WindowSystemConfig> config) 
{
    if (!glfwInit())
    {
        return;
    }
    m_Width  = config.width;
    m_Height = config.height;

    auto backend = config.rhiBackend;
    if (backend == ERHIBackend::Vulkan)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    } else
    {
        LOG_FATAL("Dont support Graphics API.");
        return;
    }
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_Handle = glfwCreateWindow(m_Width, m_Height, SSTR_TO_UTF8(config.title).c_str(), nullptr, nullptr);
    if (!m_Handle)
    {
        glfwTerminate();
        return;
    }

    // Setup input callbacks
    glfwSetWindowUserPointer(m_Handle, this);
    glfwSetKeyCallback(m_Handle, KeyCallback);
    glfwSetCharCallback(m_Handle, CharCallback);
    glfwSetCharModsCallback(m_Handle, CharModsCallback);
    glfwSetMouseButtonCallback(m_Handle, MouseButtonCallback);
    glfwSetCursorPosCallback(m_Handle, CursorPosCallback);
    glfwSetCursorEnterCallback(m_Handle, CursorEnterCallback);
    glfwSetScrollCallback(m_Handle, ScrollCallback);
    glfwSetDropCallback(m_Handle, DropCallback);
    glfwSetFramebufferSizeCallback(m_Handle, FramebufferResizeCallback);
    glfwSetWindowCloseCallback(m_Handle, WindowCloseCallback);

    glfwSetInputMode(m_Handle, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
}

void WindowSystem::Destory()
{
    glfwDestroyWindow(m_Handle);
    glfwTerminate();
}

GLFWwindow* WindowSystem::GetHandle() const
{
    return m_Handle;
}

void WindowSystem::PollEvents() const
{
    glfwPollEvents();
}

void WindowSystem::WaitEvents() const
{
    glfwWaitEvents();
}

bool WindowSystem::ShouldClose() const
{
    return glfwWindowShouldClose(m_Handle);
}

std::tuple<uint32_t, uint32_t> WindowSystem::GetWindowSize() const
{
    return std::make_tuple(m_Width, m_Height);
}

std::tuple<uint32_t, uint32_t> WindowSystem::GetFramebufferSize() const
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_Handle, &width, &height);
    return std::make_tuple(width, height);
}

bool WindowSystem::IsFramebufferResized() const
{
    return m_FramebufferResized;
}

void WindowSystem::SetFramebufferResizedToDefault()
{
    m_FramebufferResized = false;
}

bool WindowSystem::IsMouseButtonDown(int button) const
{
    if (button < GLFW_MOUSE_BUTTON_1 || button > GLFW_MOUSE_BUTTON_LAST)
    {
        return false;
    }
    return glfwGetMouseButton(m_Handle, button) == GLFW_PRESS;
}

bool WindowSystem::GetFocusMode() const
{
    return m_IsFocusMode;
}

void WindowSystem::SetFocusMode(bool mode)
{
    m_IsFocusMode = mode;
    glfwSetInputMode(m_Handle, GLFW_CURSOR, m_IsFocusMode ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void WindowSystem::SetTitle(const char* title)
{
    glfwSetWindowTitle(m_Handle, title);
}
}