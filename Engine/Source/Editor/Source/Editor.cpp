#include "Engine/Source/Editor/Include/Editor.h"
namespace Snowy::Ark
{
void Editor::Init(Engine* engine)
{
    m_Engine = engine;
}
void Editor::Run()
{
    m_Engine->Run();
}
void Editor::Destroy()
{

}
}

