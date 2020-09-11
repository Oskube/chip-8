#ifndef CHIP8_H
#define CHIP8_H

#define CHIP8_PROG_START    0x200
#define CHIP8_RAM_LEN       0xE90
#define CHIP8_PROG_MAX_LEN  ( CHIP8_RAM_LEN - CHIP8_PROG_START )
#define REGISTER_V_COUNT    15
#define REGISTER_I_LEN      2

typedef struct {
    unsigned char* ram;
    unsigned char v[REGISTER_V_COUNT]; /* General purpose registers*/
    unsigned char i[REGISTER_I_LEN];   /* Address register */
} chip8_hw;

#endif // CHIP8_H
