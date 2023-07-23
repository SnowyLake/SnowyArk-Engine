#include "Engine/Source/Runtime/Engine.h"

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