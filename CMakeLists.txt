cmake_minimum_required(VERSION 3.10)
project(chip8-emulator)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)   # for the C++ parts

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Gather all ImGui C++ sources
set(IMGUI_SOURCES
    libs/imgui/imgui.cpp
    libs/imgui/imgui_draw.cpp
    libs/imgui/imgui_tables.cpp
    libs/imgui/imgui_widgets.cpp
    libs/imgui/backends/imgui_impl_glfw.cpp
    libs/imgui/backends/imgui_impl_opengl3.cpp
)

# Gather cimgui sources (C++ glue)
set(CIMGUI_SOURCES
    libs/cimgui/cimgui.cpp
    # If cimgui provides separate backend wrappers, include them:
    libs/cimgui/backends/cimgui_impl_glfw.cpp
    libs/cimgui/backends/cimgui_impl_opengl3.cpp
)

# Combine all C++ sources into one library
add_library(imgui_cpp STATIC ${IMGUI_SOURCES} ${CIMGUI_SOURCES})
target_include_directories(imgui_cpp PRIVATE
    libs/imgui
    libs/imgui/backends
    libs/cimgui
    ${OPENGL_INCLUDE_DIR}
)
target_link_libraries(imgui_cpp ${OPENGL_gl_LIBRARY} glfw)

# C executable
add_executable(chip8-emulator
    src/main.c
    src/chip8.c
    src/chip8_ui.c
    # Any other C files...
)

target_include_directories(chip8-emulator PRIVATE
    src
    libs/cimgui          # for cimgui.h
    libs/imgui           # for backends (if needed for constants)
    ${OPENGL_INCLUDE_DIR}
)

# Link executable with the C++ library and system libs
target_link_libraries(chip8-emulator imgui_cpp ${OPENGL_gl_LIBRARY} glfw)