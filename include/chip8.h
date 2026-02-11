#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/*
    CHIP-8 struct that emulates the state of original inteprreter
*/
typedef struct {
    uint8_t memory[4096];
    bool display[2048];
    bool draw_flag; // whether should the screen be updated

    uint16_t stack[16]; // a stack that used to call subroutines/functions and return from them
    uint16_t V[16]; // general purpose variable registers for holding values at the memory; VF is a flag register

    uint16_t PC; // program counter for pointing at current instruction at the memory
    uint16_t I; // index pointer at the specific location at the memory
    uint8_t SP; // stack pointer

    // timers (both decrement at 60hz)
    uint8_t delay_timer;
    uint8_t sound_timer; // for every value bigger than 0 there shall be a beep

    // imput
    bool keys[16];

    // execution state
    bool running; // is emulation running?
    bool halted; // does emulation encountered an error? unknown opcode?
    uint64_t cycle_count; // the number of CPU cycles executed 

    // rom info
    const char rom_path[256];
    size_t rom_size; 

} Chip8;

// === INTERFACE ===

// LIFECYCLE

/*
    Creates a new instance of Chip-8.

    returns a pointer to the emulator or NULL at failure.
*/
Chip8* chip8_create(void);

/*
    Cleans the instance of Chip-8 and renders it's pointer NULL.
*/
void chip8_destroy(Chip8** chip);

/*
    Reset emulator to the initial state.
    Keep ROM loaded, reset PC to 0x200.
*/
void chip8_reset(Chip8* chip);

// ROM MANAGEMENT

/*
    Load ROM at memory at 0x200.
    Check for possible errors.
    Fill the struct info about ROM.
*/
bool chip8_load_rom(Chip8* chip, const char* path);

/*
    Resets CHIP-8.
    Reloads ROM from the path.
*/
bool chip8_reload_rom(Chip8* chip);

/*
    Returns rom path.
*/
const char* chip8_get_rom_path(Chip8* chip);

// TIMERS

/*
    Decrement both counters at 60 hz until they = 0, then restart
*/
void chip8_update_timers(Chip8* chip);

// EXECUTION CONTROL

// There shall be step, step-n and start, pause, and checkers for running and halting

/*
    Executes 1 CPU cycle.
    If succesfull - returns true, if halted - false.
*/
bool chip8_step(Chip8* chip);

/*
    Executes n CPU cycles.
    If halted - returns the instruction at which it was halted.
*/
int chip8_step_n(Chip8* chip, int n);

/*
    Starts executing cycles indefinitely until halt or error.
*/
void chip8_start(Chip8* chip);

/*
    Stops execution.
*/
void chip8_stop(Chip8* chip);

/*
    Returns status of chip->running
*/
bool chip8_is_running(Chip8* chip);

/*
    Returns status of chip->halted
*/
bool chip8_is_halted(Chip8* chip);

// INPUT MANAGEMENT

// shall include key pressed, key released, is key pressed

void chip8_key_press(Chip8* chip, bool key);

void chip8_key_release(Chip8* chip, bool key);

void chip8_is_key_pressed(Chip8* chip, bool key);

// STATE QUERIES FOR UI/DEBUGGING

uint16_t chip8_get_pc(Chip8* chip);

uint16_t chip8_get_i(Chip8* chip);

uint8_t chip8_get_register(Chip8* chip, int reg);

uint8_t chip8_get_sp(Chip8* chip);

uint16_t chip8_get_stack(Chip8* chip, int depth);

uint8_t chip8_get_delay_timer(Chip8* chip);

uint8_t chip8_get_sound_timer(Chip8* chip);

uint64_t chip8_get_cycle_count(Chip8* chip);

/*
    Get the pointer to memory (read-only). Shall not modify.
*/
const uint8_t* chip8_get_memory(Chip8* chip);

/*
    Get the pointer to display (read-only). Shall not modify, only read.
    Gets an array.
*/
const bool* chip8_get_display(Chip8* chip);

/*
    Check if display needs redraw.
    Clear the draw_flag when called.
*/
bool chip8_should_draw(Chip8* chip);

// MEMORY ACCESS

// shall read byte, write byte, read opcode

/*
    Reads byte from the address.
*/
uint8_t chip8_read_memory(Chip8* chip, uint16_t address);

/*
    Writes byte into the address.
*/
void chip8_write_memory(Chip8* chip, uint16_t address, uint8_t byte);

/*
    Reads the combined two bytes (big-endian) from this and next address.
*/
uint16_t chip8_read_opcode(Chip8* chip, uint16_t addres);

// DISSASEMBLY

/*
    Dissasemble opcode at given address.
    Shall return string like "LD V5, 0x0F"
    buffer is to write a string and it shall not be less than 64 bit.
*/
void chip8_disassemble(Chip8* chip, uint16_t address, char* buffer, size_t bufsize);

#endif