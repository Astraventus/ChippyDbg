#include "../include/chip8_log.h"
#include "../include/chip8.h"
#include <stdio.h>

static bool g_debug_enabled;
static bool g_verbose_enabled;

// PRIVATE FUNCTIONS

void chip8_log_draw_screen(Chip8* chip) {
    if (!chip) return;

    const bool* display = chip8_get_display(chip);

    for (uint16_t y = 0; y < CHIP8_DISPLAY_HEIGHT; y++) {
        printf(" ");

        for (uint16_t x = 0; x < CHIP8_DISPLAY_WIDTH; x++) {
            if (display[(y * CHIP8_DISPLAY_WIDTH + x)]) {
                printf("#");
            } else {
                printf(".");
            }
        }
        printf("\n");
    }
    fprintf(stderr, "\n");
}

// INTERFACE

void chip8_log_debug_set_enabled(bool debug) {
    g_debug_enabled = debug;
}

void chip8_log_verbose_set_enabled(bool verbose) {
    g_verbose_enabled = verbose;
}

void chip8_log_instructions(Chip8* chip, uint16_t opcode) {
    if (!chip || !g_debug_enabled) return;

    uint16_t local_cycle = chip8_get_cycle_count(chip);
    uint16_t local_pc = chip8_get_pc(chip);
    uint16_t affected_register = ((opcode & 0x0F00) >> 8);

    fprintf(stderr, "[%06llu] PC=0x%03X OP=0x%04X ", (unsigned long long)local_cycle, local_pc, opcode);

    char disasm_buffer[CHIP8_DISASM_BUFSIZE];
    chip8_disassemble(chip, local_pc, disasm_buffer, sizeof(disasm_buffer));

    if (g_verbose_enabled) {
        fprintf(stderr, " -> V%X = %02X", affected_register, chip8_get_register(chip, affected_register));
    }

    fprintf(stderr, "\n");
}

void chip8_log_registers(Chip8* chip) {
    if (!chip || !g_verbose_enabled) return;

    fprintf(stderr, " [REG] ");
    for (uint8_t i = 0; i < CHIP8_NUM_REGISTERS; i++) {
        fprintf(stderr, " V%X = %02X", i, chip8_get_register(chip, i));
    }
    fprintf(stderr, "\n");

}

void chip8_log_screen(Chip8* chip) {
    if (!chip || !g_debug_enabled) return;

    fprintf(stderr, "[DRAW] Display updated at PC = 0x%03X\n", chip8_get_pc(chip));
    fprintf(stderr, " [DISPLAY %2dx%2d]\n", CHIP8_DISPLAY_WIDTH, CHIP8_DISPLAY_HEIGHT);
    chip8_log_draw_screen(chip);
}

void chip8_log_halt(Chip8* chip, const char* reason) {
    if (!chip) return;
    fprintf(stderr, "[HALT] %s (PC=0x%03X, Cycles=%llu)\n",
            reason ? reason : "Unknown",
            chip8_get_pc(chip),
            (unsigned long long)chip8_get_cycle_count(chip));
}