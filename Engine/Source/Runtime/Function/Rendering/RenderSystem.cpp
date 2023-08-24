#include "RenderSystem.h"
namespace Snowy::Ark
{
void RenderSystem::Init(RenderSystemConfig config)
{
    m_RHIContext = RHIManager::CreateRHI(config.rhi);
    m_RHIContext->Init();
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