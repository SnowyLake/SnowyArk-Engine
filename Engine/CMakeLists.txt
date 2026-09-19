cmake_minimum_required(VERSION 3.24)

project(SnowyArk VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/ProjectOptions.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/Warnings.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/Dependencies.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/Shaders.cmake")

add_subdirectory(Source/Runtime)
add_subdirectory(Source/Editor)
add_subdirectory(Source/Tools/ShaderCompiler)

if(BUILD_TESTING)
    add_subdirectory(Tests)
endif()
