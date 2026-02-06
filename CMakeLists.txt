cmake_minimum_required(VERSION 3.12.0)
project(Opengl_game VERSION 1.0.0)

find_package(OpenGL REQUIRED)

add_executable(Opengl_game
    src/config.h
    src/config.cpp
    src/main.cpp
    src/helpers/glad.c
    src/helpers/triangle.cpp
    src/helpers/camera.cpp
    src/helpers/quad.cpp
    src/helpers/shader.cpp
    src/helpers/input.cpp

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

target_link_libraries(Opengl_game glfw OpenGL::GL)