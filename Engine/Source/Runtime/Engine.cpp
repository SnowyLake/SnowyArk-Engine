#include "Engine/Source/Runtime/Engine.h"

namespace Snowy::Ark
{
void Engine::Init(ERHIBackend type)
{
    m_RHIContext = RHIManager::CreateRHI(type);

    
}
void Engine::Run()
{
    m_RHIContext->Run();
}
void Engine::Destroy()
{
}
}