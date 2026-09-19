#pragma once

#include <cstdio>
#include <exception>
#include <utility>

namespace SnowyArk
{

class CleanupWait
{
public:
    /// Runs a GPU wait during cleanup. Never throws; writes a best-effort stderr diagnostic and returns false on failure.
    template <typename WaitFn> static bool TryWait(WaitFn&& wait) noexcept
    {
        try
        {
            std::forward<WaitFn>(wait)();
            return true;
        }
        catch (const std::exception& exception)
        {
            try
            {
                std::fputs("Failed to wait for the GPU during cleanup", stderr);
                const char* const detail = exception.what();
                if (detail != nullptr && detail[0] != '\0')
                {
                    std::fputs(": ", stderr);
                    std::fputs(detail, stderr);
                }
                std::fputc('\n', stderr);
            }
            catch (...)
            {
            }
            return false;
        }
        catch (...)
        {
            try
            {
                std::fputs("Failed to wait for the GPU during cleanup.\n", stderr);
            }
            catch (...)
            {
            }
            return false;
        }
    }
};

}
