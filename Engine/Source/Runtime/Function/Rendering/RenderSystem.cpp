#include "RenderSystem.h"
#include "Engine/Source/Runtime/Function/Global/GlobalContext.h"
namespace Snowy::Ark
{
void RenderSystem::Init(RenderSystemConfig config)
{
    m_RHIContext = RHIManager::CreateRHI(config.rhi.backend);
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