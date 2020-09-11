#include "chip8.h"
#include "opcodes.h"

typedef void (*instr_fptr)(chip8_hw* chip, unsigned opcode);

instr_fptr DecodeOpcode( unsigned );
