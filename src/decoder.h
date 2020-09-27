#include "chip8.h"
#include "opcodes.h"

#define INVALID_OPCODE 0xffff

unsigned DecodeOpcode( unsigned );
