#ifndef OPCODES_H
#define OPCODES_H

#include <stddef.h>
#include "chip8.h"

#define GET_NIBBLE(val, nib)  ((val >> (nib*4)) & 0xf)

typedef void (*instr_fptr)(chip8_hw* chip, unsigned opcode);

/* Opcodes */
void _0nnn(chip8_hw*, unsigned);
void _00E0(chip8_hw*, unsigned);
void _00EE(chip8_hw*, unsigned);
void _1nnn(chip8_hw*, unsigned);
void _2nnn(chip8_hw*, unsigned);
void _3xnn(chip8_hw*, unsigned);
void _4xnn(chip8_hw*, unsigned);
void _5xy0(chip8_hw*, unsigned);
void _6xnn(chip8_hw*, unsigned);
void _7xnn(chip8_hw*, unsigned);
void _8xy0(chip8_hw*, unsigned);
void _8xy1(chip8_hw*, unsigned);
void _8xy2(chip8_hw*, unsigned);
void _8xy3(chip8_hw*, unsigned);
void _8xy4(chip8_hw*, unsigned);
void _8xy5(chip8_hw*, unsigned);
void _8xy6(chip8_hw*, unsigned);
void _8xy7(chip8_hw*, unsigned);
void _8xyE(chip8_hw*, unsigned);
void _9xy0(chip8_hw*, unsigned);
void _Annn(chip8_hw*, unsigned);
void _Bnnn(chip8_hw*, unsigned);
void _Cxnn(chip8_hw*, unsigned);
void _Dxyn(chip8_hw*, unsigned);
void _Ex9E(chip8_hw*, unsigned);
void _ExA1(chip8_hw*, unsigned);
void _Fx07(chip8_hw*, unsigned);
void _Fx0A(chip8_hw*, unsigned);
void _Fx15(chip8_hw*, unsigned);
void _Fx18(chip8_hw*, unsigned);
void _Fx1E(chip8_hw*, unsigned);
void _Fx29(chip8_hw*, unsigned);
void _Fx33(chip8_hw*, unsigned);
void _Fx55(chip8_hw*, unsigned);
void _Fx65(chip8_hw*, unsigned);
void _invalid_op(chip8_hw*, unsigned);

typedef struct {
    unsigned mask;
    char*    fmt;
} operand;

static const operand oper_0nnn = { 0x0fff, "%d"  };
static const operand oper_00nn = { 0x00ff, "%d"  };
static const operand oper_000n = { 0x000f, "%d"  };
static const operand oper_0x00 = { 0x0f00, "V%d" };
static const operand oper_00y0 = { 0x00f0, "V%d" };
static const operand oper_i_br = { 0, "[I]" };
static const operand oper_st   = { 0, "ST" };
static const operand oper_dt   = { 0, "DT" };
static const operand oper_i    = { 0, "I"  };
static const operand oper_k    = { 0, "K"  };
static const operand oper_f    = { 0, "F"  };
static const operand oper_b    = { 0, "B"  };
static const operand oper_v0   = { 0, "V0" };
static const operand oper_null = { 0, NULL };

#define MAX_OPERANDS 3
typedef struct {
    instr_fptr     fun;      /**< Pointer to function implementing opcode */
    unsigned short base;     /**< Opcode base, which will be replaced by operands */
    const char*    mnemonic; /**< Mnemonic format */
    const operand  operands[MAX_OPERANDS];
} mnemonic;

static const mnemonic mnemonic_list[] = {
    { _00E0, 0x00e0, "CLS" , { oper_null }},
    { _00EE, 0x00ee, "RET" , { oper_null }},
    { _1nnn, 0x1000, "JMP" , { oper_0nnn, oper_null, oper_null }},
    { _2nnn, 0x2000, "CALL", { oper_0nnn, oper_null, oper_null }},
    { _3xnn, 0x3000, "SE"  , { oper_0x00, oper_00nn, oper_null }},
    { _4xnn, 0x4000, "SNE" , { oper_0x00, oper_00nn, oper_null }},
    { _5xy0, 0x5000, "SE"  , { oper_0x00, oper_00y0, oper_null }},
    { _6xnn, 0x6000, "MOV" , { oper_0x00, oper_00nn, oper_null }},
    { _7xnn, 0x7000, "ADD" , { oper_0x00, oper_00nn, oper_null }},
    { _8xy0, 0x8000, "MOV" , { oper_0x00, oper_00y0, oper_null }},
    { _8xy1, 0x8001, "OR"  , { oper_0x00, oper_00y0, oper_null }},
    { _8xy2, 0x8002, "AND" , { oper_0x00, oper_00y0, oper_null }},
    { _8xy3, 0x8003, "XOR" , { oper_0x00, oper_00y0, oper_null }},
    { _8xy4, 0x8004, "ADD" , { oper_0x00, oper_00y0, oper_null }},
    { _8xy5, 0x8005, "SUB" , { oper_0x00, oper_00y0, oper_null }},
    { _8xy6, 0x8006, "SHL" , { oper_0x00, oper_00y0, oper_null }},
    { _8xy7, 0x8007, "SUBN", { oper_0x00, oper_00y0, oper_null }},
    { _8xyE, 0x800e, "SHR" , { oper_0x00, oper_null, oper_null }}, /* y operand not used? */
    { _9xy0, 0x9000, "JNE" , { oper_0x00, oper_00y0, oper_null }},
    { _Annn, 0xa000, "MOV" , { oper_i   , oper_0nnn, oper_null }},
    { _Bnnn, 0xb000, "JMP" , { oper_v0  , oper_0nnn, oper_null }},
    { _Cxnn, 0xc000, "RND" , { oper_0x00, oper_00nn, oper_null }},
    { _Dxyn, 0xd000, "DRW" , { oper_0x00, oper_00y0, oper_000n }},
    { _Ex9E, 0xe000, "KE"  , { oper_0x00, oper_null, oper_null }},
    { _ExA1, 0xe000, "KNE" , { oper_0x00, oper_null, oper_null }},
    { _Fx07, 0xf007, "MOV" , { oper_0x00, oper_dt  , oper_null }},
    { _Fx0A, 0xf00a, "MOV" , { oper_0x00, oper_k   , oper_null }},
    { _Fx15, 0xf015, "MOV" , { oper_dt  , oper_0x00, oper_null }},
    { _Fx18, 0xf018, "MOV" , { oper_st  , oper_0x00, oper_null }},
    { _Fx1E, 0xf01e, "ADD" , { oper_i   , oper_0x00, oper_null }},
    { _Fx29, 0xf029, "MOV" , { oper_f   , oper_0x00, oper_null }},
    { _Fx33, 0xf033, "MOV" , { oper_b   , oper_0x00, oper_null }},
    { _Fx55, 0xf055, "MOV" , { oper_i_br, oper_0x00, oper_null }},
    { _Fx65, 0xf065, "MOV" , { oper_0x00, oper_i_br, oper_null }},
    { NULL }
};

unsigned GetOperandCount(const mnemonic* m);
unsigned ApplyMaskToValue(unsigned mask, unsigned value);

#endif // OPCODES_H
