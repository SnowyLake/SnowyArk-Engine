#pragma once
#include <SnowyCore/SnowyCore.hpp>
#include "Engine/Source/Runtime/Engine.h"
namespace Snowy::Ark
{
class Editor
{
public:
    Editor() = default;
    ~Editor() = default;

    void Init(ObserverHandle<Engine> engine);
    void Run();
    void Destroy();

private:
    RawHandle<Engine> m_Engine = nullptr;
};
}


