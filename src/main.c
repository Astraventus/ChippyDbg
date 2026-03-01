#define _POSIX_C_SOURCE 199309L

#include "../include/chip8.h"
#include "../include/chip8_log.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif  

void sleep_ms(uint32_t ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    struct timespec ts = {
        .tv_sec = ms / 1000,
        .tv_nsec = (ms % 1000) * 1000000
    };
    nanosleep(&ts, NULL);
#endif
}

int main(int argc, char* argv[]) {
    bool debug_mode = false;
    bool verbose_mode = false;
    bool slow_mode = false;
    const char* rom_path = NULL;

    // parse arguments
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--debug") == 0) {
            debug_mode = true;
        } 
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose_mode = true;
        }
        else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--slow") == 0) {
            slow_mode = true;
        }
        else if (argv[i][0] != '-') {
            rom_path = argv[i];
        }
    }

    // enable debug logging
    chip8_log_debug_set_enabled(debug_mode);
    chip8_log_verbose_set_enabled(verbose_mode);

    // create emulation
    Chip8* chip = chip8_create();
    if (!chip) {
        fprintf(stderr, "ERROR: Failed to create emulator!\n");
        return 1;
    }   

    // load rom
    if (rom_path) {
        if (!chip8_load_rom(chip, rom_path)) {
            chip8_destroy(&chip);
            return 1;
        }
    }

    // main loop
    while (!chip8_is_halted(chip)) {
        if (!chip8_step(chip)) {
            break;
        }

        uint16_t logged_pc = chip8_get_pc(chip);
        if (logged_pc >= 2) {
            logged_pc -= 2;
        }
        uint16_t logged_opcode = chip8_read_opcode(chip, logged_pc);

        chip8_log_instructions(chip, logged_opcode);
        chip8_log_registers(chip);

        if (chip8_should_draw(chip))
        {
            chip8_log_screen(chip);
        }

        chip8_update_timers(chip);
        
        if (slow_mode) {
            sleep_ms(1000);  // 1 second per instruction (debug)
        } else {
            sleep_ms(16);     // ~16ms = 60Hz (normal speed)
        }
    }

    
    // cleanup
    chip8_log_halt(chip, "Emulation completed!\n");
    chip8_destroy(&chip);

    return 0;
}