file(GLOB_RECURSE shaderCompilerSources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(SnowyArkShaderCompiler ${shaderCompilerSources})

target_include_directories(
    SnowyArkShaderCompiler
    PRIVATE
        "${PROJECT_SOURCE_DIR}/Source"
)

target_link_libraries(
    SnowyArkShaderCompiler
    PRIVATE
        SnowyArkProjectOptions
        SnowyArkWarnings
)
