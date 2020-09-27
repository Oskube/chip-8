#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <endian.h>

#include "chip8.h"
#include "opcodes.h"

static void GetNibbles(unsigned opc, unsigned char* arr)
{
    *arr     = opc & 0xf;
    opc >>= 4;
    *(arr+1) = opc & 0xf;
    opc >>= 4;
    *(arr+2) = opc & 0xf;
}

/* For opcodes 0xnXYn */
static void GetXY(chip8_hw* chip, unsigned op, unsigned char** x, unsigned char** y)
{
    unsigned char nibbles[3] = {0};
    GetNibbles(op, nibbles);
    *x = &(chip->V[ nibbles[2] ]);
    *y = &(chip->V[ nibbles[1] ]);
}

void _0nnn(chip8_hw* chip, unsigned opcode)
{
    _invalid_op(chip, opcode);
}

void _00E0(chip8_hw* chip, unsigned opcode)
{
    memset( chip->gfx, 0, CHPI8_GFX_LEN );
}

void _00EE(chip8_hw* chip, unsigned opcode)
{
    /* TODO: Check for stack_top == 0? */
    chip->stack_top--;
    chip->PC = chip->stack[ chip->stack_top ];
}

void _1nnn(chip8_hw* chip, unsigned opcode)
{
    chip->PC = be16toh(opcode & 0xfff);
}

void _2nnn(chip8_hw* chip, unsigned opcode)
{
    chip->stack[ chip->stack_top ] = chip->PC;
    chip->stack_top++;
}

void _3xnn(chip8_hw* chip, unsigned opcode)
{
    unsigned reg = GET_NIBBLE(opcode, 2);
    unsigned cmp = opcode & 0xff;
    if ( chip->V[reg] == cmp )
    {
        chip->PC++;
    }
}

void _4xnn(chip8_hw* chip, unsigned opcode)
{
    unsigned reg = GET_NIBBLE(opcode, 2);
    unsigned cmp = opcode & 0xff;
    if ( chip->V[reg] != cmp )
    {
        chip->PC++;
    }
}

void _5xy0(chip8_hw* chip, unsigned opcode)
{
    unsigned char nibbles[3] = {0};
    GetNibbles(opcode, nibbles);
    if (chip->V[ nibbles[1] ] == chip->V[ nibbles[2] ])
    {
        chip->PC++;
    }
}

void _6xnn(chip8_hw* chip, unsigned opcode)
{
    unsigned reg = GET_NIBBLE(opcode, 2);
    chip->V[ reg ] = opcode & 0xff;
}

void _7xnn(chip8_hw* chip, unsigned opcode)
{
    unsigned reg = GET_NIBBLE(opcode, 2);
    chip->V[ reg ] += opcode & 0xff;
}

void _8xy0(chip8_hw* chip, unsigned opcode)
{
    unsigned char* x = NULL;
    unsigned char* y = NULL;

    GetXY(chip, opcode, &x, &y);
    *x = *y;
}

void _8xy1(chip8_hw* chip, unsigned opcode)
{
    unsigned char* x = NULL;
    unsigned char* y = NULL;

    GetXY(chip, opcode, &x, &y);
    *x = *x | *y;
}

void _8xy2(chip8_hw* chip, unsigned opcode)
{
    unsigned char* x = NULL;
    unsigned char* y = NULL;

    GetXY(chip, opcode, &x, &y);
    *x = *x & *y;
}

void _8xy3(chip8_hw* chip, unsigned opcode)
{
    unsigned char* x = NULL;
    unsigned char* y = NULL;

    GetXY(chip, opcode, &x, &y);
    *x = *x ^ *y;
}

void _8xy4(chip8_hw* chip, unsigned opcode)
{
    unsigned char* x = NULL;
    unsigned char* y = NULL;

    GetXY(chip, opcode, &x, &y);
    *x += *y;

    if (*x < *y) // Overflow
    {
        chip->V[0xf] = 1;
    }
    else
    {
        chip->V[0xf] = 0;
    }
}

void _8xy5(chip8_hw* chip, unsigned opcode)
{
    unsigned char* x = NULL;
    unsigned char* y = NULL;

    if (*x < *y) // Underflow
    {
        chip->V[0xf] = 0; // Borrow
    }
    else
    {
        chip->V[0xf] = 1;
    }

    GetXY(chip, opcode, &x, &y);
    *x -= *y;
}

void _8xy6(chip8_hw* chip, unsigned opcode)
{
    unsigned char* x = NULL;
    unsigned char* y = NULL;

    GetXY(chip, opcode, &x, &y);
    chip->V[0xf] = *x & 1;
    *x = *x >> 1;
}

void _8xy7(chip8_hw* chip, unsigned opcode)
{
    unsigned char* x = NULL;
    unsigned char* y = NULL;

    if (*y < *x) // Underflow
    {
        chip->V[0xf] = 0; // Borrow
    }
    else
    {
        chip->V[0xf] = 1;
    }
    GetXY(chip, opcode, &x, &y);
    *x = *y - *x;
}

void _8xyE(chip8_hw* chip, unsigned opcode)
{
    unsigned char* x = NULL;
    unsigned char* y = NULL;

    GetXY(chip, opcode, &x, &y);
    if ((*x & 0x80) == 0)
    {
        chip->V[0xf] = 0;
    }
    else
    {
        chip->V[0xf] = 1;
    }

    *x = *x << 1;
}

void _9xy0(chip8_hw* chip, unsigned opcode)
{
    unsigned char* x = NULL;
    unsigned char* y = NULL;

    GetXY(chip, opcode, &x, &y);
    if (*x != *y)
    {
        chip->PC++;
    }
}

void _Annn(chip8_hw* chip, unsigned opcode)
{
    chip->I = be16toh(opcode && 0xfff);
}

void _Bnnn(chip8_hw* chip, unsigned opcode)
{
    chip->PC= chip->V[0] + be16toh(opcode & 0xfff); /* TODO: problems with endianness?*/
}

void _Cxnn(chip8_hw* chip, unsigned opcode)
{
    unsigned x = GET_NIBBLE(opcode, 2);
    chip->V[x] = rand() & (opcode && 0xff);
}

void _Dxyn(chip8_hw* chip, unsigned opcode)
{
    /* TODO */
}

void _Ex9E(chip8_hw* chip, unsigned opcode)
{
    unsigned x = GET_NIBBLE(opcode, 2);
    unsigned key = chip->get_key( false );
    if (key == chip->V[x])
    {
        chip->PC++;
    }
}

void _ExA1(chip8_hw* chip, unsigned opcode)
{
    unsigned x = GET_NIBBLE(opcode, 2);
    unsigned key = chip->get_key( false );
    if (key != chip->V[x])
    {
        chip->PC++;
    }
}

void _Fx07(chip8_hw* chip, unsigned opcode)
{
    unsigned x = GET_NIBBLE(opcode, 2);
    unsigned delay = chip->DT;
    chip->V[x] = delay;
} void _Fx0A(chip8_hw* chip, unsigned opcode)
{
    unsigned x = GET_NIBBLE(opcode, 2);
    unsigned key = chip->get_key( true );
    chip->V[x] = key;
}

void _Fx15(chip8_hw* chip, unsigned opcode)
{
    unsigned x = GET_NIBBLE(opcode, 2);
    chip->DT = chip->V[x];
}

void _Fx18(chip8_hw* chip, unsigned opcode)
{
    unsigned x = GET_NIBBLE(opcode, 2);
    chip->ST = chip->V[x];
}

void _Fx1E(chip8_hw* chip, unsigned opcode)
{
    unsigned char* x = NULL;
    unsigned char* y = NULL;

    GetXY(chip, opcode, &x, &y);
    chip->I += *x;
}

void _Fx29(chip8_hw* chip, unsigned opcode)
{
    /* TODO */
}

void _Fx33(chip8_hw* chip, unsigned opcode)
{
    unsigned val = chip->V[ GET_NIBBLE(opcode, 2) ];
    unsigned pos = chip->I;
    chip->ram[ pos   ] = val / 100;
    chip->ram[ pos+1 ] = (val % 100) / 10;
    chip->ram[ pos+2 ] = (val % 10);
}

void _Fx55(chip8_hw* chip, unsigned opcode)
{
    unsigned pos = chip->I;
    unsigned last = GET_NIBBLE(opcode, 2);
    for (unsigned i = 0; i <= last; i++)
    {
        chip->ram[ pos+i ] = chip->V[ i ];
    }
}

void _Fx65(chip8_hw* chip, unsigned opcode)
{
    unsigned pos = chip->I;
    unsigned last = GET_NIBBLE(opcode, 2);
    for (unsigned i = 0; i <= last; i++)
    {
        chip->V[ i ] = chip->ram[ pos+i ];
    }
}

void _invalid_op(chip8_hw* chip, unsigned opcode)
{
}

unsigned GetMnemonicCount()
{
    unsigned i = 0;
    for (; mnemonic_list[i].fun != NULL; i++);
    return i;
}

unsigned GetOperandCount(const mnemonic* m)
{
    if (!m) return 0;
    unsigned count = 0;
    while (m->operands[ count ].fmt != NULL)
    {
        count++;
        if (count >= MAX_OPERANDS) break;
    }

    return count;
}

unsigned ApplyMaskToValue(unsigned mask, unsigned value)
{
    if (mask == 0) return 0;

    /* Move mask to left in a loop until mask & 0x1 return non-zero return number of shifts */
    unsigned shift = 0;
    for (unsigned m = ~mask; m & 0x1; m >>= 1)
    {
        shift++;
    }

    return mask & (value << shift);
}

unsigned GetValueByMask(unsigned value, unsigned mask)
{
    if (mask == 0) return 0;

    /* Move mask to left in a loop until mask & 0x1 return non-zero return number of shifts */
    unsigned shift = 0;
    for (unsigned m = ~mask; m & 0x1; m >>= 1)
    {
        shift++;
    }

    return (value & mask) >> shift;
}
