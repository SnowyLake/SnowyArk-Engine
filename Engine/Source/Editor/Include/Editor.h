#pragma once
#include "Engine\Source\Runtime\Engine.h"
namespace Snowy::Ark
{
class Editor
{
public:
    Editor() = default;
    ~Editor() = default;

    void Init(Engine* engine);
    void Run();
    void Destroy();

private:
    Engine* m_Engine = nullptr;
};
}


