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
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_NO_EXCEPTIONS 1
#define VULKAN_HPP_NO_CONSTRUCTORS
// GLFW
#define GLFW_INCLUDE_VULKAN
#endif  // defined(SNOWY_ARK_RHI_VULKAN)

// GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
