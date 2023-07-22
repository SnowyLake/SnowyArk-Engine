#include "Engine/Source/Runtime/Engine.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/RHIManager.h"
namespace Snowy::Ark
{
void Engine::Init(EGraphicsBackendType type)
{
    m_RHI = RHIManager::CreateRHI(type);
}
void Engine::Run()
{
    m_RHI->Run();
}
void Engine::Destroy()
{
}
}