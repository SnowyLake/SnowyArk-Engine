#pragma once

#include <cstdint>

namespace SnowyArk
{

enum class GraphicsBackend
{
    Vulkan
};

enum class Format
{
    Unknown,
    R8G8B8A8Srgb
};

struct Extent2D
{
    uint32_t width = 0;
    uint32_t height = 0;
};

struct Color
{
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

struct Viewport
{
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float minDepth = 0.0f;
    float maxDepth = 1.0f;
};

struct Rect2D
{
    int32_t x = 0;
    int32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

}
