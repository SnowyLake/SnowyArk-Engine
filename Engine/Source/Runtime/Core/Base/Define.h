#pragma once
/* ---------------------------------------------------- */
/* SnowyArk Engine                                      */
/* ---------------------------------------------------- */
// RHI
#define SNOWY_ARK_RHI_VULKAN


/* ---------------------------------------------------- */
/* ThirdParty                                           */
/* ---------------------------------------------------- */

#if defined(SNOWY_ARK_RHI_VULKAN)
// Vulkan Core
// ------------------------------------------
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_NO_NODISCARD_WARNINGS
    // Disable Vk Result Assert
    #if !defined(VULKAN_HPP_ASSERT_ON_RESULT)
    #define VULKAN_HPP_ASSERT_ON_RESULT
    #endif
// GLFW
#define GLFW_INCLUDE_VULKAN
#endif  // defined(SNOWY_ARK_RHI_VULKAN)

// GLM
// ------------------------------------------
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

// spdlog
// ------------------------------------------
//#define SPDLOG_COMPILED_LIB
#define SPDLOG_USE_STD_FORMAT