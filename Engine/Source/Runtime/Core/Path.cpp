#include <Runtime/Core/Path.h>

#ifdef _WIN32
// WIN32_LEAN_AND_MEAN and NOMINMAX must be defined before windows.h.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

#include <Runtime/Core/Log.h>

namespace SnowyArk
{

std::filesystem::path Path::ExecutableDirectory()
{
#ifdef _WIN32
    wchar_t buffer[MAX_PATH] {};
    const DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (length == 0 || length == MAX_PATH)
    {
        Log::Fatal("Failed to query the executable path.");
    }
    return std::filesystem::path(buffer).parent_path();
#else
    Log::Fatal("Path::ExecutableDirectory is not implemented on this platform.");
#endif
}

}
