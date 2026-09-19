#pragma once

#include <cstdint>

#include <Runtime/GAL/GraphicsTypes.h>

namespace SnowyArk
{

class SwapChain
{
public:
    SwapChain(const SwapChain&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;
    virtual ~SwapChain() = default;

    /// Returns the current swapchain image extent.
    virtual Extent2D GetExtent() const = 0;

    /// Returns the swapchain color format.
    virtual Format GetColorFormat() const = 0;

    /// Recreates the swapchain for the given framebuffer size.
    virtual void Resize(uint32_t width, uint32_t height) = 0;

protected:
    SwapChain() = default;
};

}
