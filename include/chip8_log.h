#ifndef CHIP8_LOG_H
#define CHIP8_LOG_H

#include "chip8.h"


// TODO IN THE FUTURE: put logging into log files after the end of the program

// global argument bools assurance; The logging supposed to activate upon the status of two global booleans

void chip8_log_debug_set_enabled(bool debug);

void chip8_log_verbose_set_enabled(bool verbose);

// INTERFACE

// call after instruction executes

void chip8_log_instructions(Chip8* chip, uint16_t opcode);

void chip8_log_registers(Chip8* chip);

void chip8_log_screen(Chip8* chip);


// halt
void chip8_log_halt(Chip8* chip, const char* reason);

# endif