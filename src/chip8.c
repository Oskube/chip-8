#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chip8.h"
#include "decoder.h"

bool Chip8Init( chip8_hw* chip )
{
    memset( chip->V    , 0, REGISTER_V_COUNT );
    memset( chip->gfx  , 0, CHIP8_GFX_LEN    );
    memset( chip->stack, 0, sizeof(unsigned)*CHIP8_STACK_LEN);

    chip->I  = 0;
    chip->PC = 0;
    chip->DT = 0;
    chip->ST = 0;
    chip->stack_top = 0;

    chip->get_key_blocking = NULL;
    chip->is_key_down      = NULL;
    chip->draw_screen      = NULL;

    chip->log_level = 0;

    chip->ram = (unsigned char*)calloc( CHIP8_RAM_LEN, sizeof(unsigned char) );
    if ( chip->ram == NULL )
    {
        return false;
    }

    memcpy( chip->ram+CHIP8_RAM_CHARSET_BEGIN, chip8_charset, CHIP8_CHARSET_LEN );

    return true;
}

void Chip8Free( chip8_hw* chip  )
{
    if ( chip )
    {
        free(chip->ram);
        chip->ram = NULL;
    }
}

bool Chip8LoadProgram( chip8_hw* chip, const char* file )
{
    if ( chip->ram == NULL ) return false;

    FILE* prog = fopen( file, "rb" );
    if ( !prog ) return false;

    size_t bytes = fread( &(chip->ram[ CHIP8_PROG_START ]),
                          sizeof(unsigned char),
                          CHIP8_PROG_MAX_LEN, prog );
    fclose( prog );

    if ( bytes == 0 ) return false;
    chip->PC = CHIP8_PROG_START;

    return true;
}

bool Chip8Dump( chip8_hw* chip, FILE* output )
{
    for (unsigned i = 0; i < REGISTER_V_COUNT; i++)
    {
        fprintf(output, "V%u: 0x%.2x (%u)\t", i, chip->V[i], chip->V[i]);
        if ((i+1) % 4 == 0)
        {
            fprintf(output, "\n");
        }
    }

    fprintf(output, "\n\n");
    fprintf(output, "DT: 0x%.2x (%u)\n", chip->DT, chip->DT);
    fprintf(output, "ST: 0x%.2x (%u)\n", chip->ST, chip->ST);
    fprintf(output, "I : 0x%.4x (%u)\n", chip->I , chip->I );
    fprintf(output, "PC: 0x%.4x (%u)\n\n", chip->PC, chip->PC);

    fprintf(output, "STACK_TOP: %u\n", chip->stack_top);
    fprintf(output, "STACK:\n\t");
    for (unsigned i = 0; i < CHIP8_STACK_LEN; i++)
    {
        fprintf(output, "%.8x ", chip->stack[i]);
        if ((i+1) % 2 == 0) fprintf(output, "\n\t");
    }

    fprintf(output, "\nGFX:\n\t");
    for (unsigned i = 0; i < CHIP8_GFX_LEN; i++)
    {
        fprintf(output, "%.2x ", chip->gfx[i]);
        if ((i+1) % 8 == 0) fprintf(output, "\n\t");
    }

    fprintf(output, "\nRAM contents:\n");
    for (unsigned i = 0; i < CHIP8_RAM_LEN; i++)
    {
        if (i%0x10 == 0)
        {
            // new line
            fprintf(output, "\n%.4x  %.2x ", i, (unsigned char)chip->ram[i]);
        }
        else
        {
            fprintf(output, "%.2x ", (unsigned char)chip->ram[i]);
        }
    }
    fprintf(output, "\n");
    return true;
}

int Chip8Execute(chip8_hw* chip, unsigned op_count)
{
    unsigned* pc = &(chip->PC);
    for(unsigned i = op_count; i > 0; i--)
    {
        unsigned opcode = chip->ram[*pc] << 8 | chip->ram[*pc+1];
        unsigned op_index = DecodeOpcode(opcode);
        *pc += 2;

        if (op_index == INVALID_OPCODE)
        {
            fprintf(stderr, "ERROR: Invalid opcode 0x%.4x[%u] at %u\n", opcode, op_index, *pc -2);
            return -1;
        }

        if (chip->log_level >= 2)
        {
            printf("Executing opcode index: 0x%.4x[%u] (%s) at %u\n", opcode, op_index, mnemonic_list[op_index].mnemonic, *pc -2);
        }
        instr_fptr fun = mnemonic_list[op_index].fun;
        fun(chip, opcode);
    }

    return 0;
}

int Chip8ProcessTimers(chip8_hw* chip, unsigned decrement_count)
{
    if (chip->DT > 0)
    {
        if(chip->DT > decrement_count)
        {
            chip->DT -= decrement_count;
        }
        else
        {
            chip->DT = 0;
        }
    }
    if (chip->ST > 0)
    {
        if(chip->ST > decrement_count)
        {
            chip->ST -= decrement_count;
        }
        else
        {
            chip->ST = 0;
        }
    }
    return 0;
}
