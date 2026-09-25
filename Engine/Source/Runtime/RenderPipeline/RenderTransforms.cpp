#include <Runtime/RenderPipeline/RenderTransforms.h>

#include <cmath>
#include <cstddef>
#include <numbers>

#include <Runtime/Core/Log.h>

namespace SnowyArk
{
RenderTransforms RenderTransforms::Create(const float elapsedSeconds, const Extent2D extent)
{
    static_assert(sizeof(RenderTransforms) == 192);
    static_assert(offsetof(RenderTransforms, view) == 64);
    static_assert(offsetof(RenderTransforms, projection) == 128);
    if (extent.width == 0 || extent.height == 0 || !std::isfinite(elapsedSeconds))
    {
        Log::Fatal("Transforms require a nonzero extent and finite time.");
    }
    const float angle = std::fmod(elapsedSeconds, 4.0f) * (std::numbers::pi_v<float> / 2.0f);
    const float cosine = std::cos(angle);
    const float sine = std::sin(angle);
    const float inverseRoot2 = 1.0f / std::sqrt(2.0f);
    const float inverseRoot3 = 1.0f / std::sqrt(3.0f);
    const float inverseRoot6 = 1.0f / std::sqrt(6.0f);
    const float focalLength = 1.0f / std::tan(std::numbers::pi_v<float> / 8.0f);
    const float aspect = static_cast<float>(extent.width) / static_cast<float>(extent.height);
    constexpr float nearPlane = 0.1f;
    constexpr float farPlane = 10.0f;
    // This pass uses a fixed camera at (2, 2, 2), looking at the origin with Z up.
    return {
        .model = { cosine, -sine, 0, 0, sine, cosine, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },
        .view = { -inverseRoot2, inverseRoot2, 0, 0, -inverseRoot6, -inverseRoot6, 2 * inverseRoot6, 0, inverseRoot3, inverseRoot3, inverseRoot3, -2 * std::sqrt(3.0f), 0, 0, 0, 1 },
        .projection = { focalLength / aspect, 0, 0, 0, 0, focalLength, 0, 0, 0, 0, farPlane / (nearPlane - farPlane), farPlane * nearPlane / (nearPlane - farPlane), 0, 0, -1, 0 },
    };
}
}
