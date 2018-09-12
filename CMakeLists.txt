# Set CMake minimum version and CMake policy required by UrhoCommon module
cmake_minimum_required (VERSION 3.2.3)
if (COMMAND cmake_policy)
    # Libraries linked via full path no longer produce linker search paths
    cmake_policy (SET CMP0003 NEW)
    # INTERFACE_LINK_LIBRARIES defines the link interface
    cmake_policy (SET CMP0022 NEW)
    # Disallow use of the LOCATION target property - so we set to OLD as we still need it
    cmake_policy (SET CMP0026 OLD)
    # MACOSX_RPATH is enabled by default
    cmake_policy (SET CMP0042 NEW)
    # Honor the visibility properties for SHARED target types only
    cmake_policy (SET CMP0063 OLD)
endif ()
# Set project name
project (OpenSpaceProgram)
# Set CMake modules search path
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/Modules)
#http://xit0.org/2013/04/cmake-use-git-branch-and-commit-details-in-project/
# Get the current working branch
#execute_process(
#  COMMAND git rev-parse --abbrev-ref HEAD
#  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#  OUTPUT_VARIABLE GIT_BRANCH
#  OUTPUT_STRIP_TRAILING_WHITESPACE
#)
# Get the latest abbreviated commit hash of the working branch
#execute_process(
#  COMMAND git log -1 --format=%h
#  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#  OUTPUT_VARIABLE GIT_COMMIT_HASH
#  OUTPUT_STRIP_TRAILING_WHITESPACE
#)
#configure_file (
#  "config.h.in" 
#  "config.h"
#)
# Include UrhoCommon.cmake module after setting project name
include (UrhoCommon)
# Define target name
set (TARGET_NAME OSP)
set (CMAKE_CXX_STANDARD 11)
ADD_SUBDIRECTORY (src)

