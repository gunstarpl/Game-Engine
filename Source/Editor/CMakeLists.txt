#
# Copyright (c) 2018-2020 Piotr Doan. All rights reserved.
#

cmake_minimum_required(VERSION 3.16)

#
# Library
#

project(Editor)
add_library(Editor STATIC
    "../../Include/Editor/EditorSystem.hpp"
    "../../Include/Editor/EditorShell.hpp"
    "../../Include/Editor/EditorRenderer.hpp"
    "../../Include/Editor/Modules/InputManagerEditor.hpp"
    "../../Include/Editor/Modules/GameStateEditor.hpp"
    "../../Deploy/Data/Engine/Shaders/Interface.shader"
    "Precompiled.hpp"
    "EditorSystem.cpp"
    "EditorShell.cpp"
    "EditorRenderer.cpp"
    "Modules/InputManagerEditor.cpp"
    "Modules/GameStateEditor.cpp"
)

target_compile_features(Editor PUBLIC cxx_std_17)
target_include_directories(Editor PUBLIC "../../Include")
target_precompile_headers(Editor PRIVATE "Precompiled.hpp")

#
# Dependencies
#

if(NOT TARGET Common)
    add_subdirectory("../Common" "Common" EXCLUDE_FROM_ALL)
endif()

target_link_libraries(Editor PRIVATE Common)

if(NOT TARGET System)
    add_subdirectory("../System" "System" EXCLUDE_FROM_ALL)
endif()

target_link_libraries(Editor PRIVATE System)

if(NOT TARGET Game)
    add_subdirectory("../Game" "Game" EXCLUDE_FROM_ALL)
endif()

target_link_libraries(Editor PRIVATE Game)

#
# External
#

add_subdirectory("../../External/ImGui-1.62" "External/ImGui" EXCLUDE_FROM_ALL)
target_include_directories("ImGui" PUBLIC "../../External/GLEW-2.1.0/include")
target_link_libraries(Editor PUBLIC "ImGui")
set_property(TARGET "ImGui" PROPERTY FOLDER "External")
