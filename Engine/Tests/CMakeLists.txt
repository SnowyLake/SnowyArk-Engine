file(GLOB_RECURSE testSources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(SnowyArkTests ${testSources})

target_link_libraries(
    SnowyArkTests
    PRIVATE
        SnowyArk
        SnowyArkWarnings
)

add_test(NAME SnowyArkTests COMMAND SnowyArkTests)

option(SNOWYARK_GPU_TESTS "Run windowed Vulkan regression checks with CTest" OFF)
if(SNOWYARK_GPU_TESTS)
    add_test(NAME SnowyArkGpuTests COMMAND SnowyArkTests --gpu)
    set_tests_properties(SnowyArkGpuTests PROPERTIES LABELS "GPU" TIMEOUT 30 FAIL_REGULAR_EXPRESSION "Vulkan:")
endif()

add_custom_target(SnowyArkTestShaders
    COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:SnowyArkTests>/Shaders/Passes"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "$<TARGET_PROPERTY:SnowyArkTriangleShader,SNOWYARK_SHADER_OUTPUT>"
        "$<TARGET_FILE_DIR:SnowyArkTests>/Shaders/Passes/Triangle.spv"
    DEPENDS SnowyArkTriangleShader
)
add_dependencies(SnowyArkTests SnowyArkTestShaders)
