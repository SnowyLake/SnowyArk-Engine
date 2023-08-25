#include "RenderSystem.h"
#include "Engine/Source/Runtime/Function/Global/GlobalContext.h"
namespace Snowy::Ark
{
void RenderSystem::Init(RenderSystemConfig config, ObserverHandle<GLFWwindow> windowHandle)
{
    m_RHIContext = RHIManager::CreateRHI(config.rhi.backend);
    config.rhi.windowHandle = windowHandle;
    m_RHIContext->Init(config.rhi);
}
void RenderSystem::Tick()
{
    m_RHIContext->Run();
}
void RenderSystem::Destory()
{
    RHIManager::DestroyRHI(m_RHIContext);
}
}