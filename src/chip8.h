#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>

#define CHIP8_PROG_START    0x200
#define CHIP8_RAM_LEN       0xE90
#define CHIP8_PROG_MAX_LEN  ( CHIP8_RAM_LEN - CHIP8_PROG_START )

#define REGISTER_V_COUNT    15
#define CHIP8_STACK_LEN     12
#define CHIP8_GFX_W         64
#define CHIP8_GFX_H         32
#define CHPI8_GFX_LEN       ( CHIP8_GFX_W * CHIP8_GFX_H / 64 )

typedef struct {
    unsigned char* ram;
    unsigned char  V[ REGISTER_V_COUNT ]; /* General purpose registers */
    unsigned short I;   /* Address register */

    unsigned PC;      /* Program counter */
    unsigned char DT; /* Delay timer */
    unsigned char ST; /* Sound timer */
    unsigned stack[ CHIP8_STACK_LEN ];
    unsigned stack_top;
    unsigned char gfx[ CHPI8_GFX_LEN ];

    unsigned (*get_key)(bool);
} chip8_hw;

#endif // CHIP8_H
