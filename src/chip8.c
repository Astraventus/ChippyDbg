#include "chip8.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

// FONTS

static const uint8_t FONT_DATA[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// PRIVATE FUNCTIONS

static void chip8_init_state(Chip8* chip) {
    // memset the entirery of chip to 0
    memset(chip, 0, sizeof(Chip8));

    // load fonts
    memcpy(chip->memory, FONT_DATA, sizeof(FONT_DATA));

    // set pc and sp, running and halted to false, and cycle_count to 0
    chip->PC = 0x200;
    chip->SP = 0;
    chip->running = false;
    chip->halted = false;
    chip->cycle_count = 0;
}

// PUBLIC FUNCTIONS (INTERFACE)

Chip8* chip8_create(void) {
    // shall allocate the memory to the struct
    Chip8* chip = calloc(1, sizeof(Chip8));
    if (!chip) {
        fprintf(stderr, "ERROR: Failed to allocate Chip-8 emulator\n");
        return NULL;
    }
    
    chip8_init_state(chip);
    return chip;
}

void chip8_destroy(Chip8** chip_ptr) {
    if (!chip_ptr || !*chip_ptr) {
        return;
    }

    free(*chip_ptr);
    *chip_ptr = NULL;
}

void chip8_reset(Chip8* chip) {
    if (!chip) {
        return;
    }

    const char rom_path_temp[256];
    strncpy(rom_path_temp, chip->rom_path, sizeof(rom_path_temp));
    size_t rom_size_temp = chip->rom_size;

    chip8_init_state(chip);

    chip->rom_size = rom_size_temp;
    strncpy(chip->rom_path, rom_path_temp, sizeof(chip->rom_path));
}

bool chip8_load_rom(Chip8* chip, const char* path) {
    if (!chip) {
        fprintf(stderr, "ERROR: Invalid instance of Chip-8 emulator during loading of ROM\n");
        return false;
    }
    if (!path) {
        fprintf(stderr, "ERROR: Invalid or absent path to ROM during the loading of ROM\n");
        return false;
    }

    // fopen the file 
    FILE* file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "ERROR: Failed to open the ROM: %s\n", path);
        return false;
    }

    // get to know the size of rom
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // check the size of rom compared to maximal permitted size in the memory
    const long max_size = chip->memory - 0x200;
    if (file_size > max_size) {
        fprintf(stderr, "ERROR: ROM too large: %ld bytes (expected no more than %ld bytes)\n", file_size, max_size);
        return false;
    }

    // load (fread) into memory
    size_t byte_read = fread(chip->memory + 0x200, 1, file_size, file);
    fclose(file);

    if (byte_read != (size_t) file_size) {
        fprintf(stderr, "ERROR: Failed to read ROM!\n");
        return false;
    }
    
    //save rom info
    chip->rom_size = file_size;
    strncpy(chip->rom_path, path, sizeof(chip->rom_path));

    printf("Succesfully loaded ROM: %s (%ld bytes)\n", path, file_size);
    return true;
}

bool chip8_reload_rom(Chip8* chip) {
    if (!chip || chip->rom_path[0] == '\0') {
        return false;
    }

    chip8_reset(chip);
    return chip8_load_rom(chip, chip->rom_path);
}

const char* chip8_get_rom_path(Chip8* chip) {
    if (!chip) return NULL;

    return chip->rom_path;
}

void chip8_update_timers(Chip8* chip) {
    if (!chip) return;

    if (chip->delay_timer > 0) {
        chip->delay_timer--;
    }
    if (chip->sound_timer > 0) {
        chip->sound_timer--;
    }
}

// TODO 12.2.2026

bool chip8_step(Chip8* chip);

int chip8_step_n(Chip8* chip, int n);

void chip8_start(Chip8* chip);

void chip8_stop(Chip8* chip);

bool chip8_is_running(Chip8* chip);

bool chip8_is_halted(Chip8* chip);

void chip8_key_press(Chip8* chip, bool key);

void chip8_key_release(Chip8* chip, bool key);

void chip8_is_key_pressed(Chip8* chip, bool key);

// END TODO

uint16_t chip8_get_pc(Chip8* chip) {
    return chip ? chip->PC : 0;
}

uint16_t chip8_get_i(Chip8* chip) {
    return chip ? chip->I : 0;
}

uint8_t chip8_get_register(Chip8* chip, int reg) {
    if (!chip || reg < 0 || reg > CHIP8_NUM_REGISTERS) {
        return 0;
    }

    return chip->V[reg];
}

uint8_t chip8_get_sp(Chip8* chip) {
    return chip ? chip->SP : 0;
}

uint16_t chip8_get_stack(Chip8* chip, int depth) {
    if (!chip || depth < 0 || depth > CHIP8_STACK_SIZE) {
        return 0;
    }

    return chip->stack[depth];
}

uint8_t chip8_get_delay_timer(Chip8* chip) {
    return chip ? chip->delay_timer : 0;
}

uint8_t chip8_get_sound_timer(Chip8* chip) {
    return chip ? chip->sound_timer : 0;
}

uint64_t chip8_get_cycle_count(Chip8* chip) {
    return chip ? chip->cycle_count : 0;
}

const uint8_t* chip8_get_memory(Chip8* chip) {
    return chip ? chip->memory : 0;
}

const bool* chip8_get_display(Chip8* chip) {
    return chip ? chip->display : 0;
}

bool chip8_should_draw(Chip8* chip) {
    if (!chip) return false;

    bool should_draw = chip->draw_flag;
    chip-> draw_flag = false;
    return should_draw;
}

// TODO 12.02.2026

uint8_t chip8_read_memory(Chip8* chip, uint16_t address);

void chip8_write_memory(Chip8* chip, uint16_t address, uint8_t byte);

uint16_t chip8_read_opcode(Chip8* chip, uint16_t addres);

void chip8_disassemble(Chip8* chip, uint16_t address, char* buffer, size_t bufsize);

// END TODO