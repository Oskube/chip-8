#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>

#define CHIP8_PROG_START    0x200
#define CHIP8_RAM_LEN       0xE90
#define CHIP8_PROG_MAX_LEN  ( CHIP8_RAM_LEN - CHIP8_PROG_START )

#define CHIP8_CHARSET_LEN         5*16
#define CHIP8_CHAR_LEN            5
#define CHIP8_RAM_CHARSET_BEGIN   0x0

#define REGISTER_V_COUNT    15
#define CHIP8_STACK_LEN     12
#define CHIP8_GFX_W         64
#define CHIP8_GFX_H         32
#define CHIP8_GFX_LEN       ( CHIP8_GFX_W / 8 * CHIP8_GFX_H )

#define CHIP8_CPU_FREQ  500
#define CHIP8_DT_FREQ   60

typedef struct {
    unsigned char* ram;
    unsigned char  V[ REGISTER_V_COUNT ]; /* General purpose registers */
    unsigned short I;   /* Address register */

    unsigned PC;      /* Program counter */
    unsigned char DT; /* Delay timer */
    unsigned char ST; /* Sound timer */
    unsigned stack[ CHIP8_STACK_LEN ];
    unsigned stack_top;
    unsigned char gfx[ CHIP8_GFX_LEN ];

    bool     (*is_key_down)(unsigned);
    unsigned (*get_key_blocking)();
} chip8_hw;

static const char chip8_charset[ CHIP8_CHARSET_LEN ] = {
    0xf0, 0x90, 0x90, 0x90, 0xf0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xf0, 0x10, 0xf0, 0x80, 0xf0, // 2
    0xf0, 0x10, 0xf0, 0x10, 0xf0, // 3
    0x90, 0x90, 0xf0, 0x10, 0x10, // 4
    0xf0, 0x80, 0xf0, 0x10, 0xf0, // 5
    0xf0, 0x80, 0xf0, 0x90, 0xf0, // 6
    0xf0, 0x10, 0x20, 0x40, 0x40, // 7
    0xf0, 0x90, 0xf0, 0x90, 0xf0, // 8
    0xf0, 0x90, 0xf0, 0x10, 0xf0, // 9
    0xf0, 0x90, 0xf0, 0x90, 0x90, // A
    0xe0, 0x90, 0xe0, 0x90, 0xe0, // B
    0xf0, 0x80, 0x80, 0x80, 0xf0, // C
    0xe0, 0x90, 0x90, 0x90, 0xe0, // D
    0xf0, 0x80, 0xf0, 0x80, 0xf0, // E
    0xf0, 0x80, 0xf0, 0x80, 0x80, // F
};

#endif // CHIP8_H
