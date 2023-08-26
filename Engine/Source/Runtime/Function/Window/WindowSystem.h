#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Global/GlobalContextConfig.h"

#include <GLFW/glfw3.h>

#include <array>
#include <functional>

namespace Snowy::Ark
{
class WindowSystem
{
public:
    void Init(WindowSystemConfig config);
    void Destory();

    GLFWwindow* GetHandle() const;

    void PollEvents() const;
    void WaitEvents() const;
    bool ShouldClose() const;

    std::tuple<uint32_t, uint32_t> GetWindowSize() const;
    std::tuple<uint32_t, uint32_t> GetFramebufferSize() const;
    bool IsFramebufferResized() const;
    void SetFramebufferResizedToDefault();

    bool IsMouseButtonDown(int button) const;

    bool GetFocusMode() const;
    void SetFocusMode(bool mode);

    void SetTitle(const char* title);


    // Window Events
    /*------------------------------------------------------------------------------*/
    using OnResetFunc               = std::function<void()>;
    using OnKeyFunc                 = std::function<void(int, int, int, int)>;
    using OnCharFunc                = std::function<void(unsigned int)>;
    using OnCharModsFunc            = std::function<void(int, unsigned int)>;
    using OnMouseButtonFunc         = std::function<void(int, int, int)>;
    using OnCursorPosFunc           = std::function<void(double, double)>;
    using OnCursorEnterFunc         = std::function<void(int)>;
    using OnScrollFunc              = std::function<void(double, double)>;
    using OnDropFunc                = std::function<void(int, const char**)>;
    using OnFramebufferResizeFunc   = std::function<void(int, int)>;
    using OnWindowCloseFunc         = std::function<void()>;
    using OnResetFunc               = std::function<void()>;

    void RegisterOnResetFunc(OnResetFunc func) { m_OnResetFuncList.emplace_back(func); }
    void RegisterOnKeyFunc(OnKeyFunc func) { m_OnKeyFuncList.emplace_back(func); }
    void RegisterOnCharFunc(OnCharFunc func) { m_OnCharFuncList.emplace_back(func); }
    void RegisterOnCharModsFunc(OnCharModsFunc func) { m_OnCharModsFuncList.emplace_back(func); }
    void RegisterOnMouseButtOnFunc(OnMouseButtonFunc func) { m_OnMouseButtonFuncList.emplace_back(func); }
    void RegisterOnCursorPosFunc(OnCursorPosFunc func) { m_OnCursorPosFuncList.emplace_back(func); }
    void RegisterOnCursorEnterFunc(OnCursorEnterFunc func) { m_OnCursorEnterFuncList.emplace_back(func); }
    void RegisterOnScrollFunc(OnScrollFunc func) { m_OnScrollFuncList.emplace_back(func); }
    void RegisterOnDropFunc(OnDropFunc func) { m_OnDropFuncList.emplace_back(func); }
    void RegisterOnFramebufferResizeFunc(OnFramebufferResizeFunc func) { m_OnFramebufferResizeFuncList.emplace_back(func); }
    void RegisterOnWindowCloseFunc(OnWindowCloseFunc func) { m_OnWindowCloseFuncList.emplace_back(func); }

protected:
    // Window Event Callbacks
    /*------------------------------------------------------------------------------*/
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        WindowSystem* ptr = (WindowSystem*)glfwGetWindowUserPointer(window);
        if (ptr)
        {
            ptr->OnKey(key, scancode, action, mods);
        }
    }
    static void CharCallback(GLFWwindow* window, unsigned int codepoint)
    {
        WindowSystem* ptr = (WindowSystem*)glfwGetWindowUserPointer(window);
        if (ptr)
        {
            ptr->OnChar(codepoint);
        }
    }
    static void CharModsCallback(GLFWwindow* window, unsigned int codepoint, int mods)
    {
        WindowSystem* ptr = (WindowSystem*)glfwGetWindowUserPointer(window);
        if (ptr)
        {
            ptr->OnCharMods(codepoint, mods);
        }
    }
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        WindowSystem* ptr = (WindowSystem*)glfwGetWindowUserPointer(window);
        if (ptr)
        {
            ptr->OnMouseButton(button, action, mods);
        }
    }
    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        WindowSystem* ptr = (WindowSystem*)glfwGetWindowUserPointer(window);
        if (ptr)
        {
            ptr->OnCursorPos(xpos, ypos);
        }
    }
    static void CursorEnterCallback(GLFWwindow* window, int entered)
    {
        WindowSystem* ptr = (WindowSystem*)glfwGetWindowUserPointer(window);
        if (ptr)
        {
            ptr->OnCursorEnter(entered);
        }
    }
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        WindowSystem* ptr = (WindowSystem*)glfwGetWindowUserPointer(window);
        if (ptr)
        {
            ptr->OnScroll(xoffset, yoffset);
        }
    }
    static void DropCallback(GLFWwindow* window, int count, const char** paths)
    {
        WindowSystem* ptr = (WindowSystem*)glfwGetWindowUserPointer(window);
        if (ptr)
        {
            ptr->OnDrop(count, paths);
        }
    }
    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        WindowSystem* ptr = (WindowSystem*)glfwGetWindowUserPointer(window);
        if (ptr)
        {
            ptr->m_Width  = width;
            ptr->m_Height = height;
            ptr->m_FramebufferResized = true;
            ptr->OnFramebufferResize(width, height);
        }
    }
    static void WindowCloseCallback(GLFWwindow* window)
    { 
        glfwSetWindowShouldClose(window, true);
    }

    void OnReset()
    {
        for (auto& func : m_OnResetFuncList)
            func();
    }
    void OnKey(int key, int scancode, int action, int mods)
    {
        for (auto& func : m_OnKeyFuncList)
            func(key, scancode, action, mods);
    }
    void OnChar(unsigned int codepoint)
    {
        for (auto& func : m_OnCharFuncList)
            func(codepoint);
    }
    void OnCharMods(int codepoint, unsigned int mods)
    {
        for (auto& func : m_OnCharModsFuncList)
            func(codepoint, mods);
    }
    void OnMouseButton(int button, int action, int mods)
    {
        for (auto& func : m_OnMouseButtonFuncList)
            func(button, action, mods);
    }
    void OnCursorPos(double xpos, double ypos)
    {
        for (auto& func : m_OnCursorPosFuncList)
            func(xpos, ypos);
    }
    void OnCursorEnter(int entered)
    {
        for (auto& func : m_OnCursorEnterFuncList)
            func(entered);
    }
    void OnScroll(double xoffset, double yoffset)
    {
        for (auto& func : m_OnScrollFuncList)
            func(xoffset, yoffset);
    }
    void OnDrop(int count, const char** paths)
    {
        for (auto& func : m_OnDropFuncList)
            func(count, paths);
    }
    void OnFramebufferResize(int width, int height)
    {
        for (auto& func : m_OnFramebufferResizeFuncList)
            func(width, height);
    }

private:
    GLFWwindow* m_Handle;
    uint32_t m_Width;
    uint32_t m_Height;
    bool m_IsFocusMode = false;
    bool m_FramebufferResized = false;

    // Window Events
    /*------------------------------------------------------------------------------*/
    std::vector<OnResetFunc>                m_OnResetFuncList;
    std::vector<OnKeyFunc>                  m_OnKeyFuncList;
    std::vector<OnCharFunc>                 m_OnCharFuncList;
    std::vector<OnCharModsFunc>             m_OnCharModsFuncList;
    std::vector<OnMouseButtonFunc>          m_OnMouseButtonFuncList;
    std::vector<OnCursorPosFunc>            m_OnCursorPosFuncList;
    std::vector<OnCursorEnterFunc>          m_OnCursorEnterFuncList;
    std::vector<OnScrollFunc>               m_OnScrollFuncList;
    std::vector<OnDropFunc>                 m_OnDropFuncList;
    std::vector<OnFramebufferResizeFunc>    m_OnFramebufferResizeFuncList;
    std::vector<OnWindowCloseFunc>          m_OnWindowCloseFuncList;
};
}
