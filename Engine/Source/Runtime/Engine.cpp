#include "Engine/Source/Runtime/Engine.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/RHIUtils.h"
namespace Snowy::Ark
{
void Engine::Init(EGraphicsBackendType type)
{
    m_RHI = RHIUtils::CreateRHI(type);
}
void Engine::Run()
{
    m_RHI->Run();
}
void Engine::Destroy()
{
}
}