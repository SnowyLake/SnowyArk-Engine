#include "Engine/Source/Editor/Include/Editor.h"
namespace Snowy::Ark
{
void Editor::Init(UniqueHandle<Engine>&& runtime)
{
    m_Runtime = std::forward<UniqueHandle<Engine>>(runtime);
}
void Editor::Run()
{
    m_Runtime->Run();
}
void Editor::Destroy()
{
    m_Runtime->Destroy();
}
}
