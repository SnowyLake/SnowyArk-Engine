# ponytail: Target-local globbing adds a build-time scan; use explicit lists if scan cost or generator support becomes a problem.
file(GLOB_RECURSE snowyArkSources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

if(NOT WIN32)
    list(FILTER snowyArkSources EXCLUDE REGEX "/Platform/Windows/")
endif()

add_library(SnowyArk STATIC ${snowyArkSources})

target_include_directories(
    SnowyArk
    PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Source>"
)

target_link_libraries(
    SnowyArk
    PUBLIC
        SnowyArkProjectOptions
    PRIVATE
        glfw
        Vulkan::Vulkan
        SnowyArkWarnings
)

target_compile_definitions(
    SnowyArk
    PRIVATE
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
        VK_USE_PLATFORM_WIN32_KHR
        GLFW_INCLUDE_NONE
)
