#include "../include/chip8_ui.h"

#include <cstdint>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// === PRIVATE FUNCTIONS ===

static GLuint create_display_texture() {
    GLuint tex;
    glGenTextures(1, &tex);

    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CHIP8_DISPLAY_WIDTH, CHIP8_DISPLAY_HEIGHT, 0, GL_RGBA,  GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);

    return tex;
}

// === PUBLIC API ===

Chip8UI* chip8_ui_create() {
    
    Chip8UI* ui = (Chip8UI*)calloc(1, sizeof(Chip8UI));

    if (!ui) {
        fprintf(stderr, "chip8_ui_create: Allocation failed!\n");
        return nullptr;
    }

    ui->chip = nullptr;
    ui->running = false;

    ui->display_texture = create_display_texture();
    ui->display_scale = 10.0f;

    ui->cycles_per_frame = 10;
    ui->step_count = 10;

    ui->memory_cols = 8;
    ui->follow_pc = true;

    ui->show_controls = true;
    ui->show_cpu_state = true;
    ui->show_display = true;
    ui->show_dissasembly = true;
    ui->show_keyboard = true;
    ui->show_memory = true;

    return ui;
}

void chip8_ui_destroy(Chip8UI** ui_ptr) {
    if (!*ui_ptr || !ui_ptr) return;

    Chip8UI* ui = *ui_ptr;
    chip8_ui_close_rom(ui);
    glDeleteTextures(1, &ui->display_texture);
    free(ui);
    *ui_ptr = nullptr;
}

bool chip8_ui_load_rom(Chip8UI* ui, const char* path) {
    if (!ui || path) return false;
    chip8_ui_close_rom(ui);

    ui->chip = chip8_create();
    if (!ui->chip) {
        fprintf(stderr, "chip8_ui_load_rom: failed to create chip8 instance\n");
        return false;
    }

    if (!chip8_load_rom(ui->chip, path)) {
        fprintf(stderr, "chip8_ui_load_rom: Failed to load ROM into chip8 instance\n");
        chip8_destroy(&ui->chip);
        return false;
    }

    strncpy(ui->rom_path, path, sizeof(ui->rom_path) - 1);
    ui->running = false;
    return true;
}

void chip8_ui_close_rom(Chip8UI* ui) {
    if (!ui) return;

    if (ui->chip) {
        chip8_destroy(&ui->chip);
    }
    ui->running = false;
    ui->rom_path[0] = '\0';
}