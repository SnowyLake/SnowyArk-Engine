#include "Engine/Source/Editor/Include/Editor.h"
namespace Snowy::Ark
{
void Editor::Init(UniqueHandle<Engine>&& engine)
{
    m_Engine = std::forward<UniqueHandle<Engine>>(engine);
}
void Editor::Run()
{
    m_Engine->Run();
}
void Editor::Destroy()
{
    m_Engine->Destroy();
}
}

