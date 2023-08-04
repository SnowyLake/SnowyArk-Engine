#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/RHIManager.h"
namespace Snowy::Ark
{
class Engine
{
public:
    void Init(ERHIBackend type);
    void Run();
    void Destroy();
    
private:
    SharedHandle<RHI> m_RHIContext;
};
}


