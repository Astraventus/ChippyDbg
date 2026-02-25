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

static uint16_t chip8_fetch(Chip8* chip) {
    uint8_t byte1 = chip->memory[chip->PC];
    uint8_t byte2 = chip->memory[chip->PC + 1];

    uint16_t opcode = (byte1 << 8) | byte2;
    return opcode;
}

static void chip8_execute(Chip8* chip, uint16_t opcode) {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t n = (opcode & 0x000F);
    uint8_t nn = (opcode & 0x00FF);
    uint16_t nnn = (opcode & 0x0FFF);

    switch (opcode && 0xF000) {
        // TODO: Implement all 35 opcode

        // === System & Flow Control (0xxx) ===
        case 0x0000:
            switch (opcode && 0x00FF) {
                case 0x00E0:
                // Clear the screen
                memset(chip->display, 0, sizeof(chip->display));
                chip->draw_flag = true;
                break;

                case 0x00EE:
                // Return from subroutine
                // Pop adress from stack and jump to it
                if (chip->SP == 0) {
                    fprintf(stderr, "ERROR: Stack Underflow!\n");
                    chip->halted = true;
                    break;
                }
                chip->SP--;
                chip->PC = chip->stack[chip->SP];
                return; 

                default: 
                /*
                0x0NNN case, 
                existed for calling original RCA1802 routines  
                Now typically deprecated in modern emulators
                */
                fprintf(stderr, "Warning: 0NNN machine code call ignored: 0x%04X\n", opcode);
                break;
            }
        
        // === Jumps & Calls (1xxx, 2xxx, Bxxx) ===
        case 0x1000:
        // Jump to address NNN
        chip->PC = nnn;
        return;

        case 0x2000:
        // Execute subroutine starting at address NNN
        if (chip->SP >= 16) {
            fprintf(stderr, "ERROR: Stack Overflow!\n");
            chip->halted = true;
            break;
        }

        chip->stack[chip->SP] = chip->PC + 2;
        chip->SP++;
        chip->PC = nnn;
        return;

        case 0xB000:
        // Original CHIP-8 uses V0; some modern variants use VX. Most ROMs expect V0.
        // Jump to address NNN + V0
        chip->PC = nnn + chip->V[0];
        return;

        //  === Skip Instructions (3xxx, 4xxx, 5xxx, 9xxx, Exxx) ===

        case 0x3000:
        // Skip the following instruction if the value of register VX equals NN
        if (chip->V[x] == nn) {
            chip->PC += 2;
        }
        break;

        case 0x4000:
        // Skip the following instruction if the value of register VX is not equal to NN
        if (chip->V[x] != nn) {
            chip->PC += 2;
        }
        break;

        case 0x5000:
        // Skip the following instruction if the value of register VX is equal to the value of register VY
        if ((opcode && 0x000F) == 0) {
            if (chip->V[x] == chip->V[y]) {
                chip->PC += 2;
            }
        }
        break;

        case 0x9000:
        // Skip the following instruction if the value of register VX is not equal to the value of register VY
        if ((opcode && 0x000F) == 0) {
            if (chip->V[x] != chip->V[y]) {
                chip->PC += 2;
            }
        }
        break;

        case 0xE000:
        switch (nn) {
            // Skip the following instruction if the key corresponding to the hex value currently stored in register VX is pressed
            case 0x9E:
            if (chip->V[x] < 16 && chip->keys[chip->V[x]]) {
                chip->PC += 2;
            }
            break;
            
            // Skip the following instruction if the key corresponding to the hex value currently stored in register VX is not pressed
            case 0xA1:
            if (chip->V[x] < 16 && !chip->keys[chip->V[x]]) {
                chip->PC += 2;
            }
            break;
        }
        break;

        // === Register Operations (6xxx, 7xxx, 8xxx) ===

        case 0x6000:
        // Store number NN in register VX
        chip->V[x] = nn;
        break;

        case 0x7000:
        // Add the value NN to register VX
        chip->V[x] += nn;
        break;

        // === ALU Operations (8XYn) ===

        case 0x8000:
        switch (n) {
            case 0x0:
            // Store the value of register VY in register VX
            chip->V[x] = chip->V[y];
            break;

            case 0x1:
            // Set VX to VX OR VY
            chip->V[x] |= chip->V[y];
            chip->V[0xF] = 0;  // VF reset (quirk)
            break;

            case 0x2:
            // Set VX to VX AND VY
            chip->V[x] &= chip->V[y];
            chip->V[0xF] = 0;  // VF reset (quirk)
            break;

            case 0x3:
            // Set VX to VX XOR VY
            chip->V[x] ^= chip->V[y];
            chip->V[0xF] = 0;  // VF reset (quirk)
            break;


            case 0x4:
            // Add the value of register VY to register VX
            // Set VF to 01 if a carry occurs
            // Set VF to 00 if a carry does not occur
            uint16_t sum = chip->V[x] + chip->V[y];
            chip->V[0xF] = (sum > 0xFF) ? 1 : 0;
            chip->V[x] = sum & 0xFF; 
            break;

            case 0x5:
            // Subtract the value of register VY from register VX
            // Set VF to 00 if a borrow occurs
            // Set VF to 01 if a borrow does not occur
            chip->V[0xF] = (chip->V[x] >= chip->V[y]) ? 1 : 0;  // NOT borrow
            chip->V[x] -= chip->V[y];
            break;

            case 0x6:
            // Store the value of register VY shifted right one bit in register VX¹
            // Set register VF to the least significant bit prior to the shift
            // VY is unchanged
            chip->V[0xF] = chip->V[y] & 0x01;
            chip->V[x] = chip->V[y] >> 1;
            break;

            case 0x7:
            // Set register VX to the value of VY minus VX
            // Set VF to 00 if a borrow occurs
            // Set VF to 01 if a borrow does not occur
            chip->V[0xF] = (chip->V[y] >= chip->V[x]) ? 1 : 0;  // NOT borrow
            chip->V[x] = chip->V[y] - chip->V[x];
            break;


            case 0xE:
            // Store the value of register VY shifted left one bit in register VX¹
            // Set register VF to the most significant bit prior to the shift
            // VY is unchanged
            chip->V[0xF] = (chip->V[y] & 0x80) >> 7;  // MSB before shift
            chip->V[x] = chip->V[y] << 1;
            break;
        }
        
        // === Memory Operations (Axxx, Fxxx) ===

        case 0xA000:
        // Store memory address NNN in register I
        chip->I = nnn;
        break;

        case 0xF000:
        switch (nn) {
            case 0x07:
            // Store the current value of the delay timer in register VX
            chip->V[x] = chip->delay_timer;
            break;

            // === Input (FX0A) ===
            case 0x0A:
            // Wait for a keypress and store the result in register VX
            bool key_pressed = false;
            for (uint8_t i = 0; i < 16; i++) {
                if (chip->keys[i]) {
                    chip->V[x] = i;
                    key_pressed = true;
                    break;
                }

                if (!key_pressed) {
                    chip->PC -= 2;
                }
            }
            break;
            

            case 0x15:
            // Set the delay timer to the value of register VX
            chip->delay_timer = chip->V[x];
            break;

            case 0x18:
            // Set the sound timer to the value of register VX
            chip->sound_timer = chip->V[x];
            break;

            case 0x1E:
            // Add the value stored in register VX to register I
            chip->I += chip->V[x];
            break;

            case 0x29:
            // Set I to the memory address of the sprite data corresponding to the hexadecimal digit stored in register VX
            chip->I = (chip->V[x] & 0x0F) * 5;
            break;

            case 0x33:
            // Store the binary-coded decimal equivalent of the value stored in register VX at addresses I, I + 1, and I + 2
            uint8_t value = chip->V[x];
            chip->memory[chip->I] = value / 100; // hundreds
            chip->memory[chip->I + 1] = (value / 10) % 10; // tens
            chip->memory[chip->I + 2] = value % 10; // ones
            break;

            case 0x55:
            // Store the values of registers V0 to VX inclusive in memory starting at address I. I is set to I + X + 1 after operation
            for (uint8_t i = 0; i <= x; i++) {
                chip->memory[chip->I + i] = chip->V[i];
            }
            chip->I += x + 1;
            break;

            case 0x65:
            // Fill registers V0 to VX inclusive with the values stored in memory starting at address I. I is set to I + X + 1 after operation
            for (uint8_t i = 0; i <= x; i++) {
                chip->V[i] = chip->memory[chip->I + i];
            }
            chip->I += x + 1;
            break;

        }

        case 0xC000:
        // Set VX to a random number with a mask of NN
        chip->V[x] = (rand() % 256) & nn;
        break;

        case 0xD000:
            {
                uint8_t lx = chip->V[x] % 64;
                uint8_t ly = chip->V[y] % 32;
                uint8_t height = chip->V[n];
                chip->V[0xF] = 0;
                for (int row = 0; row < height; row++) {
                    uint8_t sprite_byte = chip->memory[chip->I + row];

                    for (uint8_t col = 0; col < 8; col++) {
                        if ((sprite_byte && (0x80 >> col) != 0)) {
                            int px = (lx + col) % 64;
                            int py = (ly + row) % 32;
                            int index = py * 64 + px;
                    
                            if (chip->display[index]) {
                                chip->V[0xF] = 1;
                            }
                            
                            chip->display[index] ^= true;
                        }
                    }
                }
                chip->draw_flag = true;
            }

        default:
            fprintf(stderr, "Unkown opcode: 0x%04X\n", opcode);
            chip->halted = true;
            break;
    }

    chip->PC += 2;
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

    char rom_path_temp[256];
    strncpy(rom_path_temp, chip->rom_path, sizeof(rom_path_temp) - 1);
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

bool chip8_step(Chip8* chip) {
    if (!chip || chip->halted) {
        return false;
    }

    uint16_t opcode = chip8_fetch(chip);
    chip8_execute(chip, opcode);
    chip->cycle_count++;

    return !chip->halted;
}

int chip8_step_n(Chip8* chip, int n) {
    if (!chip) return 0;

    int executed = 0;
    for (int i = 0; i < n && !chip->halted; i++) {
        chip8_step(chip);
        executed++;
    }

    return executed;
}

void chip8_start(Chip8* chip) {
    if (!chip) return false;
    chip->running = true;
}

void chip8_stop(Chip8* chip) {
    if (!chip) return false;
    chip->running = false;
}

bool chip8_is_running(Chip8* chip) {
    return chip && chip->running;
}

bool chip8_is_halted(Chip8* chip) {
    return chip && chip->halted;
}

void chip8_key_press(Chip8* chip, uint8_t key) {
    if (!chip || key >= CHIP8_NUM_KEYS) return;

    chip->keys[key] = true;
}

void chip8_key_release(Chip8* chip, uint8_t key) {
    if (!chip || key >= CHIP8_NUM_KEYS) return;

    chip->keys[key] = false;
}

bool chip8_is_key_pressed(Chip8* chip, uint8_t key) {
    if (!chip || key >= CHIP8_NUM_KEYS) return false;

    return chip->keys[key];
}

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

uint8_t chip8_read_memory(Chip8* chip, uint16_t address) {
    if (!chip || address >= CHIP8_MEMORY_SIZE) return 0;

    return chip->memory[address];
}

void chip8_write_memory(Chip8* chip, uint16_t address, uint8_t byte) {
    if (!chip || address >= CHIP8_MEMORY_SIZE) return 0;

    chip->memory[address] = byte;
}

uint16_t chip8_read_opcode(Chip8* chip, uint16_t address) {
    if (!chip || address >= CHIP8_MEMORY_SIZE) return 0;

    uint8_t byte1 = chip->memory[address];
    uint8_t byte2 = chip->memory[address + 1];
    
    return (byte1 << 8) || byte2;
}

void chip8_disassemble(Chip8* chip, uint16_t address, char* buffer, size_t bufsize) {
    if (!chip || !buffer || bufsize == 0) {
        if (buffer && bufsize > 0) buffer[0] = '\0';
        return;
    }
    
    // Bounds check - need at least 2 bytes for an opcode
    if (address >= CHIP8_MEMORY_SIZE - 1) {
        snprintf(buffer, bufsize, "0x0000: OUT_OF_BOUNDS");
        return;
    }
    
    uint16_t opcode = chip8_read_opcode(chip, address);
    
    // Extract common components
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t n = (opcode & 0x000F);
    uint8_t nn = (opcode & 0x00FF);
    uint16_t nnn = (opcode & 0x0FFF);
    
    switch (opcode & 0xF000) {
        // === 0xxx: System & Flow Control ===
        case 0x0000:
            switch (opcode & 0x00FF) {
                case 0x00E0:
                    snprintf(buffer, bufsize, "0x%04X: CLS", opcode);
                    break;
                case 0x00EE:
                    snprintf(buffer, bufsize, "0x%04X: RET", opcode);
                    break;
                default:
                    // 0NNN - deprecated SYS call
                    snprintf(buffer, bufsize, "0x%04X: SYS 0x%03X", opcode, nnn);
                    break;
            }
            break;
            
        // === 1xxx: Jump ===
        case 0x1000:
            snprintf(buffer, bufsize, "0x%04X: JP 0x%03X", opcode, nnn);
            break;
            
        // === 2xxx: Call Subroutine ===
        case 0x2000:
            snprintf(buffer, bufsize, "0x%04X: CALL 0x%03X", opcode, nnn);
            break;
            
        // === 3xxx: Skip if Equal (Immediate) ===
        case 0x3000:
            snprintf(buffer, bufsize, "0x%04X: SE V%X, 0x%02X", opcode, x, nn);
            break;
            
        // === 4xxx: Skip if Not Equal (Immediate) ===
        case 0x4000:
            snprintf(buffer, bufsize, "0x%04X: SNE V%X, 0x%02X", opcode, x, nn);
            break;
            
        // === 5xxx: Skip if Equal (Register) ===
        case 0x5000:
            if ((opcode & 0x000F) == 0) {
                snprintf(buffer, bufsize, "0x%04X: SE V%X, V%X", opcode, x, y);
            } else {
                snprintf(buffer, bufsize, "0x%04X: UNKNOWN_5XY%X", opcode, n);
            }
            break;
            
        // === 6xxx: Load Immediate ===
        case 0x6000:
            snprintf(buffer, bufsize, "0x%04X: LD V%X, 0x%02X", opcode, x, nn);
            break;
            
        // === 7xxx: Add Immediate ===
        case 0x7000:
            snprintf(buffer, bufsize, "0x%04X: ADD V%X, 0x%02X", opcode, x, nn);
            break;
            
        // === 8xxx: ALU Operations ===
        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0:
                    snprintf(buffer, bufsize, "0x%04X: LD V%X, V%X", opcode, x, y);
                    break;
                case 0x1:
                    snprintf(buffer, bufsize, "0x%04X: OR V%X, V%X", opcode, x, y);
                    break;
                case 0x2:
                    snprintf(buffer, bufsize, "0x%04X: AND V%X, V%X", opcode, x, y);
                    break;
                case 0x3:
                    snprintf(buffer, bufsize, "0x%04X: XOR V%X, V%X", opcode, x, y);
                    break;
                case 0x4:
                    snprintf(buffer, bufsize, "0x%04X: ADD V%X, V%X", opcode, x, y);
                    break;
                case 0x5:
                    snprintf(buffer, bufsize, "0x%04X: SUB V%X, V%X", opcode, x, y);
                    break;
                case 0x6:
                    snprintf(buffer, bufsize, "0x%04X: SHR V%X {, V%X}", opcode, x, y);
                    break;
                case 0x7:
                    snprintf(buffer, bufsize, "0x%04X: SUBN V%X, V%X", opcode, x, y);
                    break;
                case 0xE:
                    snprintf(buffer, bufsize, "0x%04X: SHL V%X {, V%X}", opcode, x, y);
                    break;
                default:
                    snprintf(buffer, bufsize, "0x%04X: UNKNOWN_8XY%X", opcode, n);
                    break;
            }
            break;
            
        // === 9xxx: Skip if Not Equal (Register) ===
        case 0x9000:
            if ((opcode & 0x000F) == 0) {
                snprintf(buffer, bufsize, "0x%04X: SNE V%X, V%X", opcode, x, y);
            } else {
                snprintf(buffer, bufsize, "0x%04X: UNKNOWN_9XY%X", opcode, n);
            }
            break;
            
        // === Axxx: Load Index ===
        case 0xA000:
            snprintf(buffer, bufsize, "0x%04X: LD I, 0x%03X", opcode, nnn);
            break;
            
        // === Bxxx: Jump with Offset ===
        case 0xB000:
            snprintf(buffer, bufsize, "0x%04X: JP V0, 0x%03X", opcode, nnn);
            break;
            
        // === Cxxx: Random ===
        case 0xC000:
            snprintf(buffer, bufsize, "0x%04X: RND V%X, 0x%02X", opcode, x, nn);
            break;
            
        // === Dxxx: Draw Sprite ===
        case 0xD000:
            snprintf(buffer, bufsize, "0x%04X: DRW V%X, V%X, 0x%X", opcode, x, y, n);
            break;
            
        // === Exxx: Keypad Skip ===
        case 0xE000:
            switch (opcode & 0x00FF) {
                case 0x9E:
                    snprintf(buffer, bufsize, "0x%04X: SKP V%X", opcode, x);
                    break;
                case 0xA1:
                    snprintf(buffer, bufsize, "0x%04X: SKNP V%X", opcode, x);
                    break;
                default:
                    snprintf(buffer, bufsize, "0x%04X: UNKNOWN_EX%X", opcode, nn);
                    break;
            }
            break;
            
        // === Fxxx: Misc Operations ===
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x07:
                    snprintf(buffer, bufsize, "0x%04X: LD V%X, DT", opcode, x);
                    break;
                case 0x0A:
                    snprintf(buffer, bufsize, "0x%04X: LD V%X, K", opcode, x);
                    break;
                case 0x15:
                    snprintf(buffer, bufsize, "0x%04X: LD DT, V%X", opcode, x);
                    break;
                case 0x18:
                    snprintf(buffer, bufsize, "0x%04X: LD ST, V%X", opcode, x);
                    break;
                case 0x1E:
                    snprintf(buffer, bufsize, "0x%04X: ADD I, V%X", opcode, x);
                    break;
                case 0x29:
                    snprintf(buffer, bufsize, "0x%04X: LD F, V%X", opcode, x);
                    break;
                case 0x33:
                    snprintf(buffer, bufsize, "0x%04X: LD B, V%X", opcode, x);
                    break;
                case 0x55:
                    snprintf(buffer, bufsize, "0x%04X: LD [I], V%X", opcode, x);
                    break;
                case 0x65:
                    snprintf(buffer, bufsize, "0x%04X: LD V%X, [I]", opcode, x);
                    break;
                default:
                    snprintf(buffer, bufsize, "0x%04X: UNKNOWN_FX%X", opcode, nn);
                    break;
            }
            break;
            
        // === Unknown ===
        default:
            snprintf(buffer, bufsize, "0x%04X: UNKNOWN", opcode);
            break;
    }
}
