function(snowyark_add_slang_shader targetName)
    cmake_parse_arguments(SHADER "" "OUTPUT_NAME" "SOURCES" ${ARGN})
    if(NOT SHADER_OUTPUT_NAME)
        set(SHADER_OUTPUT_NAME "${targetName}.spv")
    endif()

    set(outputDir "${CMAKE_CURRENT_BINARY_DIR}/Shaders/Passes")
    set(outputFile "${outputDir}/${SHADER_OUTPUT_NAME}")

    add_custom_command(
        OUTPUT "${outputFile}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${outputDir}"
        COMMAND "${SNOWYARK_SLANGC}"
            ${SHADER_SOURCES}
            -target spirv
            -profile spirv_1_4
            -emit-spirv-directly
            -fvk-use-entrypoint-name
            -entry MainVertex
            -entry MainFragment
            -o "${outputFile}"
        DEPENDS ${SHADER_SOURCES}
        COMMENT "Compiling Slang shader ${SHADER_OUTPUT_NAME}"
        VERBATIM
    )

    add_custom_target("${targetName}" DEPENDS "${outputFile}")
    set_property(TARGET "${targetName}" PROPERTY SNOWYARK_SHADER_OUTPUT "${outputFile}")
endfunction()
