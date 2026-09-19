#pragma once

namespace SnowyArk
{

class EditorApplication
{
public:
    /// Creates the editor window and runs the main loop, waiting for events while the framebuffer is zero.
    int Run();
};

}
