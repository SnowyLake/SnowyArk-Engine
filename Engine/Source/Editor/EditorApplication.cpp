#include <Editor/EditorApplication.h>

#include <Runtime/Application/Application.h>
#include <Runtime/Core/Log.h>
#include <Runtime/Platform/Window.h>

namespace SnowyArk
{

int EditorApplication::Run()
{
    Window window;
    WindowDesc desc;
    desc.title = "SnowyArk";
    desc.width = 1280;
    desc.height = 720;
    desc.resizable = true;
    if (!window.Create(desc))
    {
        return 1;
    }

    Application application;
    if (!application.Initialize(window))
    {
        Log::Error("Failed to initialize the runtime application.");
        return 1;
    }

    while (!window.ShouldClose())
    {
        if (window.HasDrawableExtent())
        {
            window.PumpEvents();
        }
        else
        {
            window.WaitEvents();
        }
        application.Tick();
    }

    application.Shutdown();
    return 0;
}

}
