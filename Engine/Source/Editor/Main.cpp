#include <exception>

#include <Editor/EditorApplication.h>
#include <Runtime/Core/Log.h>

/// Runs the SnowyArk editor executable.
int main()
{
    try
    {
        SnowyArk::EditorApplication editor;
        return editor.Run();
    }
    catch (const std::exception& exception)
    {
        SnowyArk::Log::Error("Unhandled exception: {}", exception.what());
        return 1;
    }
}
