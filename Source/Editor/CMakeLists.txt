#
# Copyright (c) 2018-2021 Piotr Doan. All rights reserved.
# Software distributed under the permissive MIT License.
#

cmake_minimum_required(VERSION 3.16)
include_guard(GLOBAL)

#
# Files
#

set(INCLUDE_FILES
    "EditorSystem.hpp"
    "EditorSubsystem.hpp"
    "EditorShell.hpp"
    "EditorModule.hpp"
    "EditorConsole.hpp"
    "EditorRenderer.hpp"
    "Modules/InputManagerEditor.hpp"
    "Modules/GameInstanceEditor.hpp"
)

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../Include/Editor/")
list(TRANSFORM INCLUDE_FILES PREPEND ${INCLUDE_DIR})
source_group(TREE ${INCLUDE_DIR} PREFIX "Header Files" FILES ${INCLUDE_FILES})

set(SOURCE_FILES
    "Precompiled.hpp"
    "EditorSystem.cpp"
    "EditorShell.cpp"
    "EditorConsole.cpp"
    "EditorRenderer.cpp"
    "Modules/InputManagerEditor.cpp"
    "Modules/GameInstanceEditor.cpp"
)

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/")
list(TRANSFORM SOURCE_FILES PREPEND ${SOURCE_DIR})
source_group(TREE ${SOURCE_DIR} PREFIX "Source Files" FILES ${SOURCE_FILES})

set(RESOURCE_FILES
    "Shaders/Interface.shader"
)

set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../Deploy/Data/Engine/")
list(TRANSFORM RESOURCE_FILES PREPEND ${RESOURCE_DIR})
source_group(TREE ${RESOURCE_DIR} PREFIX "Resource Files" FILES ${RESOURCE_FILES})

#
# Library
#

project(Editor)
add_library(Editor ${INCLUDE_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})
target_compile_features(Editor PUBLIC cxx_std_17)
target_include_directories(Editor PUBLIC "../../Include" "../../Source")
target_precompile_headers(Editor PRIVATE "Precompiled.hpp")
set_property(TARGET Editor PROPERTY FOLDER "Engine")

#
# Dependencies
#

add_subdirectory("../Core" "Core")
target_link_libraries(Editor PRIVATE Core)

add_subdirectory("../System" "System")
target_link_libraries(Editor PRIVATE System)

add_subdirectory("../Game" "Game")
target_link_libraries(Editor PRIVATE Game)

enable_reflection(Editor ${INCLUDE_DIR} ${SOURCE_DIR})
