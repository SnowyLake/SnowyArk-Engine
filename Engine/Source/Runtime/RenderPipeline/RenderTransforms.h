#pragma once

#include <Runtime/GAL/GraphicsTypes.h>

namespace SnowyArk
{
struct alignas(16) RenderTransforms
{
    float model[16];
    float view[16];
    float projection[16];

    /// Builds row-major matrices for the rectangle pass: Z rotation, a fixed camera, and right-handed zero-to-one depth.
    /// Throws for a zero extent or non-finite time. Y is flipped by the viewport, not by these matrices.
    static RenderTransforms Create(float elapsedSeconds, Extent2D extent);
};
}
