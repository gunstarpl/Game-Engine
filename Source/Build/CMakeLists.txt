#
# Copyright (c) 2018-2020 Piotr Doan. All rights reserved.
#

cmake_minimum_required(VERSION 3.16)

#
# Library
#

add_library(Build STATIC
    "../../Include/Build/Build.hpp"
    
    "Build.cpp"
)

target_compile_features(Build PUBLIC cxx_std_17)
target_include_directories(Build PUBLIC "../../Include")
target_include_directories(Build PRIVATE "../../Include/Build")

#
# Dependencies
#

add_subdirectory("../Common" "Common" EXCLUDE_FROM_ALL)
target_link_libraries(Build PUBLIC Common)

#
# Config
#

# Run Git commands to retrieve and save information about current build version.
find_package("Git")

add_custom_target("BuildInfo"
    COMMAND ${CMAKE_COMMAND}
        -D GIT_EXECUTABLE="${GIT_EXECUTABLE}"
        -D ENGINE_PROJECT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../"
        -D GAME_PROJECT_DIR="${CMAKE_SOURCE_DIR}"
        -D INPUT_FILE="${CMAKE_CURRENT_SOURCE_DIR}/BuildInfo.hpp.in"
        -D OUTPUT_FILE="${CMAKE_CURRENT_BINARY_DIR}/BuildInfo.hpp"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/BuildInfo.cmake"
)

add_dependencies(Build "BuildInfo")

# Add file with generated build version to engine sources.
# Only the output header file is needed for the build process.
# We have to create a dummy file as it will not be generated yet.
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/BuildInfo.hpp")
target_sources(Build PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/BuildInfo.hpp")
target_include_directories(Build PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")