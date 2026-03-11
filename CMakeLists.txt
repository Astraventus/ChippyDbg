cmake_minimum_required(VERSION 3.16)
project(ChippyDbg)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)

add_executable(chip8dbg
    src/main.cpp
    src/chip8.c
    src/chip8_log.c
    src/chip8_ui.cpp
    libs/glad/src/glad.c
    libs/tinyfiledialogs/tinyfiledialogs.c
    libs/imgui/imgui.cpp
    libs/imgui/imgui_draw.cpp
    libs/imgui/imgui_tables.cpp
    libs/imgui/imgui_widgets.cpp
    libs/imgui/imgui_demo.cpp
    libs/imgui/backends/imgui_impl_glfw.cpp
    libs/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(chip8dbg PRIVATE
    include
    libs/imgui
    libs/imgui/backends
    libs/glad/include
    libs/tinyfiledialogs
    ${GLFW_INCLUDE_DIRS}
)

target_compile_definitions(chip8dbg PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)
target_link_libraries(chip8dbg PRIVATE ${GLFW_LIBRARIES} GL dl m)
set_target_properties(chip8dbg PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})