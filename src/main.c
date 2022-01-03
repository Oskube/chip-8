#include <stdio.h>
#include <time.h>

#include "raylib_ui.h"
#include "chip8.h"

#define SECOND_IN_NSEC  1000000000

unsigned GetStepsFromTimestamps(struct timespec* begin, struct timespec* end, unsigned freq);
void PrintCounters(struct timespec* begin, struct timespec* end, unsigned ops, unsigned timer);

const char* help_text = \
"./emulator <path-to-chip8-bin> [-v[v]]\n"
"\t-v\tVerbose output\n"
"\t-vv\tMore verbose output\n";

int main( int argc, char** argv )
{
    if (argc < 2)
    {
        fprintf(stderr, "Path to chip8 program is missing!\n%s", help_text);
        return -3;
    }

    chip8_hw chip8;
    Chip8Init( &chip8 );
    const char* prog_path = argv[1]; // "testi.bin"
    if (!Chip8LoadProgram(&chip8, prog_path)) // "test.bin")
    {
        fprintf(stderr, "Failed to load program %s\n%s", prog_path, help_text);
        return -2;
    }

    if (argc > 2)
    {
        char* arg = argv[2];
        if (arg[0] == '-' && arg[1] == 'v')
        {
            chip8.log_level = 1;
            if(arg[2] == 'v')
            {
                chip8.log_level = 2;
            }
        }
        else
        {
            printf("Invalid arguments!\n%s", help_text);
            Chip8Free( &chip8 );
            return -3;
        }
    }

    chip8.get_key_blocking = RlGetKeyBlocking;
    chip8.is_key_down      = RlIsKeyDown;
    chip8.draw_screen      = RlDrawScreen;

    RlInitializeWindow(10, "Chip8 - Emulator");

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
            if (chip8.was_blocking) break;
        }

        unsigned timer_steps = GetStepsFromTimestamps(&(ts[1]), &ts_now, CHIP8_DT_FREQ);
        timer_count += timer_steps;
        if (timer_steps > 0)
        {
            // Decrement delay and sound timers
            Chip8ProcessTimers(&chip8, timer_steps);
            if (chip8.log_level >= 1)
            {
                PrintCounters(&ts_begin, &ts_now, ops_count, timer_count);
            }
        }

        if (chip8.was_blocking)
        {
            chip8.was_blocking = false;
            clock_gettime( CLOCK_MONOTONIC, &(ts[0]) );
        }
    }
    Chip8Dump( &chip8, stdout );

    RlClose();
    Chip8Free( &chip8 );

    return 0;
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
