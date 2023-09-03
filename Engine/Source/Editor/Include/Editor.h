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

    void Init(UniqueHandle<Engine>&& engine);
    void Run();
    void Destroy();

private:
    UniqueHandle<Engine> m_Engine = nullptr;
};
}


