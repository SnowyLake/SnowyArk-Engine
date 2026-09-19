#pragma once

// WIN32_LEAN_AND_MEAN and NOMINMAX must be defined before windows.h. vulkan_raii.hpp follows so Win32 surface types are already visible.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
// clang-format off
#include <windows.h>

#include <vulkan/vulkan_raii.hpp>
// clang-format on
