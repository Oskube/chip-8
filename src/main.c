#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "raylib_ui.h"
#include "chip8.h"
#include "decoder.h"

#define SECOND_IN_NSEC  1000000000

bool Chip8Init( chip8_hw* chip );
void Chip8Free( chip8_hw* chip );
bool Chip8LoadProgram( chip8_hw* chip, const char* file );
bool Chip8Dump( chip8_hw* chip, FILE* output );

int Chip8Execute(chip8_hw* chip, unsigned op_count);
int Chip8ProcessTimers(chip8_hw* chip, unsigned decrement_count);
unsigned GetStepsFromTimestamps(struct timespec* begin, struct timespec* end, unsigned freq);
void PrintCounters(struct timespec* begin, struct timespec* end, unsigned ops, unsigned timer);

int main( int argc, char** argv )
{
    chip8_hw chip8;
    Chip8Init( &chip8 );

    chip8.get_key_blocking = RlGetKeyBlocking;
    chip8.is_key_down      = RlIsKeyDown;

    RlInitializeWindow(10, "Chip8 - Emulator");
    /*
    // Test pattern
    for(unsigned i = 0; i < CHIP8_GFX_LEN; i++)
    {
        unsigned char pattern = 0xaa;
        if (i * 8 / CHIP8_GFX_W % 2 == 0) pattern = 0x55;
        chip8.gfx[ i ] = pattern;
    }
    */


    //const char* prog_path = "priv_IBM Logo.ch8"; // "test.bin")
    const char* prog_path = "testi.bin";
    if (!Chip8LoadProgram(&chip8, prog_path)) // "test.bin")
    {
        fprintf(stderr, "Failed to load program %s\n", prog_path);
        return -2;
    }

    const unsigned cpu_freq = CHIP8_CPU_FREQ;
    struct timespec ts_begin = {0};
    struct timespec ts[2] = { 0 };
    clock_gettime( CLOCK_MONOTONIC, &ts_begin );
    ts[0] = ts_begin;
    ts[1] = ts_begin;

    unsigned timer_count = 0,
             ops_count   = 0;

    while(!RlShouldQuit())
    {
        RlDrawScreen(&chip8);

        struct timespec ts_now = {0};
        clock_gettime( CLOCK_MONOTONIC, &ts_now );

        unsigned pending_ops = GetStepsFromTimestamps(&(ts[0]), &ts_now, cpu_freq);
        ops_count += pending_ops;
        for (; pending_ops > 0; pending_ops--)
        {
            // Process opcodes
            Chip8Execute(&chip8, pending_ops);
        }

        unsigned timer_steps = GetStepsFromTimestamps(&(ts[1]), &ts_now, CHIP8_DT_FREQ);
        timer_count += timer_steps;
        if (timer_steps > 0)
        {
            // Decrement delay and sound timers
            Chip8ProcessTimers(&chip8, timer_steps);
            PrintCounters(&ts_begin, &ts_now, ops_count, timer_count);
        }
    }
    Chip8Dump( &chip8, stdout );

    RlClose();
    Chip8Free( &chip8 );

    return 0;
}

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

unsigned GetStepsFromTimestamps(struct timespec* begin, struct timespec* end, unsigned freq)
{
    unsigned step = 0;
    long step_nsec = SECOND_IN_NSEC / freq;

    time_t d_sec  = end->tv_sec  - begin->tv_sec;
    long   d_nsec = end->tv_nsec - begin->tv_nsec;
    if (d_nsec < 0)
    {
        d_sec--;
        begin->tv_sec++;
        d_nsec = SECOND_IN_NSEC + d_nsec;
    }

    while (1)
    {
        while (d_nsec > step_nsec)
        {
            d_nsec -= step_nsec;
            step++;
        }
        if (d_sec == 0)
        {
            begin->tv_nsec = end->tv_nsec - d_nsec;
            return step;
        }

        d_sec--;
        begin->tv_sec++;
        d_nsec += SECOND_IN_NSEC;
    }
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

        printf("Executing opcode index: 0x%.4x[%u] (%s) at %u\n", opcode, op_index, mnemonic_list[op_index].mnemonic, *pc -2);
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

void PrintCounters(struct timespec* begin, struct timespec* end, unsigned ops, unsigned timer)
{
    time_t d_sec  = end->tv_sec  - begin->tv_sec;
    long   d_nsec = end->tv_nsec - begin->tv_nsec;

    if (d_nsec < 0)
    {
        d_sec--;
        d_nsec = SECOND_IN_NSEC + d_nsec;
    }

    float sec = d_sec + (float)d_nsec/SECOND_IN_NSEC;
    printf("OPS/s: %f(%u), Timer/s: %f(%u), sec: %f\n", (float)ops/sec, ops, (float)timer/sec, timer, sec);
}
