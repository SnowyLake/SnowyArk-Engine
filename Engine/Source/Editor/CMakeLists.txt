file(GLOB_RECURSE editorSources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(SnowyArkEditor ${editorSources})

target_link_libraries(
    SnowyArkEditor
    PRIVATE
        SnowyArk
        SnowyArkWarnings
)

snowyark_add_slang_shader(SnowyArkTriangleShader
    OUTPUT_NAME Triangle.spv
    SOURCES "${PROJECT_SOURCE_DIR}/Assets/Shaders/Passes/Triangle.slang"
)
add_dependencies(SnowyArkEditor SnowyArkTriangleShader)

add_custom_command(
    TARGET SnowyArkEditor
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:SnowyArkEditor>/Shaders/Passes"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "$<TARGET_PROPERTY:SnowyArkTriangleShader,SNOWYARK_SHADER_OUTPUT>"
        "$<TARGET_FILE_DIR:SnowyArkEditor>/Shaders/Passes/Triangle.spv"
    COMMENT "Copying compiled triangle shader next to SnowyArkEditor"
)
