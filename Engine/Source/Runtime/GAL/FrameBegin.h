#pragma once

#include <cstdint>

namespace SnowyArk
{

enum class SwapChainAcquireStatus
{
    Success,
    Suboptimal,
    OutOfDate
};

struct SwapChainAcquireResult
{
    SwapChainAcquireStatus status = SwapChainAcquireStatus::OutOfDate;
    uint32_t imageIndex = 0;
};

struct AcquiredFrameAction
{
    bool recordAndPresent = false;
    bool requestRecreation = false;
    uint32_t imageIndex = 0;

    /// Decides record/present and recreation for a single acquire result without retrying acquire.
    [[nodiscard]] static AcquiredFrameAction Decide(const SwapChainAcquireStatus status, const uint32_t imageIndex)
    {
        switch (status)
        {
        case SwapChainAcquireStatus::Success:
            return { .recordAndPresent = true, .requestRecreation = false, .imageIndex = imageIndex };
        case SwapChainAcquireStatus::Suboptimal:
            return { .recordAndPresent = true, .requestRecreation = true, .imageIndex = imageIndex };
        case SwapChainAcquireStatus::OutOfDate:
            return { .recordAndPresent = false, .requestRecreation = true, .imageIndex = 0 };
        }
        return {};
    }
};

}
