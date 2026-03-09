#pragma once

#include <stdbool.h>

#include "chip8.h"

#include "../libs/glad/include/glad/glad.h"
#include <GLFW/glfw3.h>
#include "../libs/imgui/imgui.h"
#include "../libs/imgui/backends/imgui_impl_glfw.h"
#include "../libs/imgui/backends/imgui_impl_opengl3.h"

typedef struct {
    // emulator
    Chip8* chip;
    bool running;
    char rom_path[256];

    // display
    GLuint display_texture; // gl texture for chip-8 screen
    float display_scale;    // zoom level for screen

    // execution control
    int cycles_per_frame;   // number of chip8_step() executions per frame
    int step_count;         // for "step_n"

    // memory and dissasembly viewer
    int memory_cols;        // bytes per row in memory viewer
    bool follow_pc;          // whether to follow pc in dissasembler or not 
    
    // windows visibility
    bool show_controls;
    bool show_memory;
    bool show_display;
    bool show_cpu_state;
    bool show_keyboard;
    bool show_dissasembly;
} Chip8UI;

// == Lifecycle ==

/*
    Allocate nd initialize UI.
    No ROM loaded yet.
*/
Chip8UI* chip8_ui_create(void);

/*
    Destroy the UI and emulator it owns.
*/
void chip8_ui_destroy(Chip8UI** ui_ptr);

// == ROM management ==

/*
    load ROM at path, replacing any ROM it currently has. 
*/
bool chip8_ui_load_rom(Chip8UI* ui, const char* path);

/*
    unload current ROM, destroy the emulator.
*/
void chip8_ui_close_rom(Chip8UI* ui);


// == Per-frame calls ==

/*
    Step emulator N steps.
    Call BEFORE Imgui::NewFrame();
*/
void chip8_ui_update(Chip8UI* ui);

/*
    Render all debugger windows.
    Call After Imgui::NewFrame();
*/
void chip8_ui_render(Chip8UI* ui);

/*
    Map physical keyboard -> CHIP8 keyboard state through GLFW state.
*/
void chip8_ui_process_keyboard(Chip8UI ui, GLFWwindow* window);