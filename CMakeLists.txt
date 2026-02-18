cmake_minimum_required(VERSION 3.12.0)
project(Opengl_game VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)

FetchContent_MakeAvailable(glfw)

find_package(OpenGL REQUIRED)

add_executable(Opengl_game
    src/config.h
    src/config.cpp
    src/main.cpp
    src/helpers/glad.c
    src/helpers/camera.cpp
    src/helpers/shader.cpp
    src/helpers/input.cpp
    src/helpers/player.cpp
    src/helpers/plane.cpp
    src/helpers/cube.cpp

    dependencies/imgui/imgui.cpp
    dependencies/imgui/imgui_draw.cpp
    dependencies/imgui/imgui_tables.cpp
    dependencies/imgui/imgui_widgets.cpp
    dependencies/imgui/imgui_demo.cpp  
    dependencies/imgui/backends/imgui_impl_glfw.cpp
    dependencies/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(Opengl_game PRIVATE 
    dependencies
    src
    src/helpers
    dependencies/imgui
    dependencies/imgui/backends
)

target_compile_definitions(Opengl_game PRIVATE ROOT_FOLDER="${CMAKE_SOURCE_DIR}")


if (WIN32)
    target_link_libraries(Opengl_game glfw opengl32)
else()
    target_link_libraries(Opengl_game glfw OpenGL::GL)
endif()