# Set CMake minimum version and CMake policy required by UrhoCommon module
cmake_minimum_required (VERSION 3.2.3)

add_subdirectory (3rdparty)

# Set project name
project (OpenSpaceProgram)

# Define target name
set (TARGET_NAME OSP)
set (CMAKE_CXX_STANDARD 11)
add_subdirectory (src)

