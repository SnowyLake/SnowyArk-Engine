find_package(glfw3 CONFIG REQUIRED)
# Header package version. Runtime still needs a 1.4 loader and GPU; see Building.md.
find_package(Vulkan 1.4 REQUIRED)

find_program(SNOWYARK_SLANGC slangc HINTS "$ENV{VULKAN_SDK}/Bin" "$ENV{VULKAN_SDK}/bin" REQUIRED)
if(NOT SNOWYARK_SLANGC)
    message(FATAL_ERROR "slangc was not found. Install the LunarG Vulkan SDK and restart the terminal so VULKAN_SDK is set.")
endif()
