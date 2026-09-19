#include <Runtime/Application/Application.h>

#include <memory>

#include <Runtime/Core/CleanupWait.h>
#include <Runtime/Core/Log.h>
#include <Runtime/GAL/CommandBuffer.h>
#include <Runtime/GAL/GraphicsDevice.h>
#include <Runtime/GAL/SwapChain.h>
#include <Runtime/Platform/Window.h>

namespace SnowyArk
{

Application::Application() = default;

Application::~Application() noexcept
{
    Shutdown();
}

bool Application::Initialize(Window& window)
{
    if (m_Window != nullptr || m_Device != nullptr || m_SwapChain != nullptr)
    {
        return false;
    }

    m_Window = &window;
    m_Device = GraphicsDevice::Create(GraphicsBackend::Vulkan);
    if (m_Device == nullptr)
    {
        Shutdown();
        return false;
    }
    if (!m_Device->Initialize(window.GetNativeHandle(), window.GetInstanceExtensions()))
    {
        Shutdown();
        return false;
    }

    m_SwapChain = m_Device->CreateSwapChain(window);
    if (m_SwapChain == nullptr)
    {
        Shutdown();
        return false;
    }
    if (!m_ShaderLibrary.Load("Passes/Triangle.spv"))
    {
        Shutdown();
        return false;
    }
    if (!m_RenderPipeline.Initialize(*m_Device, m_ShaderLibrary, *m_SwapChain))
    {
        Shutdown();
        return false;
    }

    Log::Info("Runtime application initialized.");
    return true;
}

void Application::Tick()
{
    if (m_Window == nullptr || m_Device == nullptr || m_SwapChain == nullptr)
    {
        return;
    }
    if (m_Window->GetWidth() == 0 || m_Window->GetHeight() == 0)
    {
        return;
    }
    if (m_Window->ConsumeResized())
    {
        m_SwapChain->Resize(m_Window->GetWidth(), m_Window->GetHeight());
    }

    CommandBuffer* const commandBuffer = m_Device->BeginFrame(*m_SwapChain);
    if (commandBuffer == nullptr)
    {
        return;
    }
    m_RenderPipeline.Render(*commandBuffer, *m_SwapChain);
    m_Device->EndFrame(*m_SwapChain);
}

void Application::Shutdown() noexcept
{
    if (m_Device != nullptr)
    {
        CleanupWait::TryWait([this] { m_Device->WaitIdle(); });
    }
    m_RenderPipeline.Shutdown();
    m_SwapChain.reset();
    if (m_Device != nullptr)
    {
        m_Device->Shutdown();
        m_Device.reset();
    }
    m_Window = nullptr;
}

}
