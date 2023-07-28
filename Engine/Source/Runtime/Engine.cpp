#include "Engine/Source/Runtime/Engine.h"

namespace Snowy::Ark
{
void Engine::Init(ERHIBackend type)
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