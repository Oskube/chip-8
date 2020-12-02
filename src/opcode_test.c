#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opcodes.h"
#include "chip8.h"

#define DEBUG_PRINT( fmt, ... )  fprintf(stderr, "\t\t" fmt, __VA_ARGS__)
//#define DEBUG_PRINT( fmt, ... )

/* Load values to V registers to be tested */
static void SetVnToValues(chip8_hw* chip, const unsigned char* values);

static int test_00E0(chip8_hw*);
static int test_call_stack(chip8_hw*);
static int test_1nnn(chip8_hw*);
static int test_3xnn(chip8_hw*);
static int test_4xnn(chip8_hw*);
static int test_5xy0(chip8_hw*);
static int test_6xnn(chip8_hw*);
static int test_7xnn(chip8_hw*);

static int test_8xy0(chip8_hw*);
static int test_8xy1(chip8_hw*);
static int test_8xy2(chip8_hw*);
static int test_8xy3(chip8_hw*);
static int test_8xy4(chip8_hw*);
static int test_8xy5(chip8_hw*);
static int test_8xy6(chip8_hw*);
static int test_8xy7(chip8_hw*);
static int test_8xyE(chip8_hw*);
static int test_9xy0(chip8_hw*);

typedef int (*test_fptr)(chip8_hw*);
typedef struct {
    test_fptr   test_fun;
    const char* name;
} test_entry;

const test_entry tests[] =
{
    { test_00E0, "Opcode 00E0"},
    { test_call_stack, "Opcodes 00EE & 2nnn" },
    { test_1nnn, "Opcode 1nnn" },
    { test_3xnn, "Opcode 3xnn" },
    { test_4xnn, "Opcode 4xnn" },
    { test_5xy0, "Opcode 5xy0" },
    { test_6xnn, "Opcode 6xnn" },
    { test_7xnn, "Opcode 7xnn" },

    { test_8xy0, "Opcode 8xy0" }, /* 8xy- tests */
    { test_8xy1, "Opcode 8xy1" },
    { test_8xy2, "Opcode 8xy2" },
    { test_8xy3, "Opcode 8xy3" },
    { test_8xy4, "Opcode 8xy4" },
    { test_8xy5, "Opcode 8xy5" },
    { test_8xy6, "Opcode 8xy6" },
    { test_8xy7, "Opcode 8xy7" },
    { test_8xyE, "Opcode 8xyE" },
    { test_9xy0, "Opcode 9xy0" },

    { NULL, NULL },
};

static const unsigned char values_1[ REGISTER_V_COUNT ] = {
    0x00, 0x22, 0x33, 0x44, 0x55,
    0x66, 0x77, 0x88, 0x99, 0xaa,
    0xbb, 0xcc, 0xdd, 0xee, 0xff
};

int main()
{
    chip8_hw chip;
    for (unsigned cur_test = 0; 1; ++cur_test)
    {
        const test_entry* cur = &(tests[ cur_test ]);
        if (cur->test_fun == NULL) break;

        Chip8Init( &chip );
        printf("Running test %d: '%s'\n", cur_test, cur->name);
        int test_res = cur->test_fun( &chip );
        if ( test_res != 0)
        {
            printf("\tTEST FAILED! %d, result: %d\n", cur_test+1, test_res);
            Chip8Dump( &chip, stdout );
            return (cur_test+1);
        }
        else
        {
            printf("\tTEST SUCCESS\n");
        }
        Chip8Free( &chip );
    }
    return 0;
}

void SetVnToValues(chip8_hw* chip, const unsigned char* values)
{
    for (unsigned i = 0; i < REGISTER_V_COUNT; ++i)
    {
        chip->V[ i ] = values[ i ];
    }
}

int test_00E0(chip8_hw* chip)
{
    memset( chip->gfx, 0xff, CHIP8_GFX_LEN );

    _00E0(chip, 0x00e0);
    for (unsigned i = 0; i < CHIP8_GFX_LEN; ++i)
    {
        if (chip->gfx[i] != 0)
        {
            return 1;
        }
    }

    return 0;
}

int test_call_stack(chip8_hw* chip)
{
    unsigned calls[CHIP8_STACK_LEN] = {0};
    for (unsigned i = 0; i < CHIP8_STACK_LEN; ++i)
    {
        calls[i] = (rand() & 0xfff) % CHIP8_RAM_LEN;
        _2nnn(chip,  0x2000 | calls[i]);
        if (chip->PC != calls[i])
        {
            return -1;
        }
    }

    for (unsigned i = CHIP8_STACK_LEN; i >= 1; --i)
    {
        if (chip->PC != calls[i-1])
        {
            DEBUG_PRINT("Expect %d != %d\n", chip->PC, calls[i-1]);
            return -2;
        }

        _00EE(chip, 0x00ee);
    }
    if (chip->PC != 0) return -3;

    return 0;
}

int test_1nnn(chip8_hw* chip)
{
    for (unsigned i = 0; i < 10; ++i)
    {
        unsigned addr = (rand() & 0xfff) % CHIP8_RAM_LEN;
        _1nnn(chip, 0x1000 | addr);
        if (chip->PC != addr)
        {
            return -1;
        }
    }
    return 0;
}

int test_3xnn(chip8_hw* chip)
{
    SetVnToValues(chip, values_1);

    const unsigned expected = 2;
    for (unsigned i = 0; i < REGISTER_V_COUNT; ++i)
    {
        for (unsigned j = 0; j < REGISTER_V_COUNT; ++j)
        {
            unsigned opcode = 0x3000 | values_1[i];
            opcode |= (j << 8);
            _3xnn(chip, opcode);
        }

        if (chip->PC != expected)
        {
            DEBUG_PRINT("Expect %d != %d (i=%u)\n", chip->PC, expected, i);
            return -1;
        }
        chip->PC = 0;
    }

    // None should match
    for (unsigned j = 0; j < REGISTER_V_COUNT; ++j)
    {
        unsigned opcode = 0x3000 | 0x11;
        opcode |= (j << 8);
        _3xnn(chip, opcode);
    }
    if (chip->PC != 0)
    {
        DEBUG_PRINT("Expect %d != %d\n", chip->PC, 0);
        return -2;
    }

    return 0;
}

int test_4xnn(chip8_hw* chip)
{
    SetVnToValues(chip, values_1);

    const unsigned expected = REGISTER_V_COUNT * 2 - 2;
    for (unsigned i = 0; i < REGISTER_V_COUNT; ++i)
    {
        for (unsigned j = 0; j < REGISTER_V_COUNT; ++j)
        {
            unsigned opcode = 0x4000 | values_1[i];
            opcode |= (j << 8);
            _4xnn(chip, opcode);
        }

        if (chip->PC != expected)
        {
            DEBUG_PRINT("Expect %d != %d (i=%u)\n", chip->PC, expected, i);
            return -1;
        }
        chip->PC = 0;
    }

    // None should match
    for (unsigned j = 0; j < REGISTER_V_COUNT; ++j)
    {
        unsigned opcode = 0x4000 | 0x11;
        opcode |= (j << 8);
        _3xnn(chip, opcode);
    }
    if (chip->PC != 0)
    {
        DEBUG_PRINT("Expect %d != %d\n", chip->PC, 0);
        return -2;
    }

    return 0;
}

int test_5xy0(chip8_hw* chip)
{
    SetVnToValues(chip, values_1);

    const unsigned expected = 2;
    for (unsigned i = 0; i < REGISTER_V_COUNT; ++i)
    {
        for (unsigned j = 0; j < REGISTER_V_COUNT; ++j)
        {
            unsigned opcode = 0x5000;
            opcode |= (j << 8);
            opcode |= (i << 4);
            _5xy0(chip, opcode);
        }

        if (chip->PC != expected)
        {
            DEBUG_PRINT("Expect %d != %d (i=%u)\n", chip->PC, expected, i);
            return -1;
        }
        chip->PC = 0;
    }
    return 0;
}

int test_6xnn(chip8_hw* chip)
{
    for (unsigned i=0; i < REGISTER_V_COUNT; ++i)
    {
        unsigned opcode = 0x6000 | i << 8 | values_1[ i ];
        _6xnn(chip, opcode);
        if (chip->V[ i ] != values_1[ i ])
        {
            DEBUG_PRINT("Expect V[ %u ], %u == %u\n", i, chip->V[i], values_1[ i ]);
            return -1;
        }
    }

    return 0;
}

int test_7xnn(chip8_hw* chip)
{
    for (unsigned i=0; i < REGISTER_V_COUNT; ++i)
    {
        unsigned sum = 0;
        unsigned value = values_1[i];
        unsigned opcode = 0x7000 | i << 8 | value;
        for (unsigned j=0; j < 5; ++j)
        {
            _7xnn(chip, opcode);
            sum += value;
            if (chip->V[ i ] != (sum & 0xff))
            {
                DEBUG_PRINT("Expect j:%u V[ %u ], %u == %u(%u)\n", j, i, chip->V[i], sum & 0xff, sum);
                return -1;
            }
        }
    }
    return 0;
}

//
// Tests for opcodes 0x8---
//
typedef int (*fptr_8nnn_test)(chip8_hw*, unsigned, unsigned);
static int test_8xxx(chip8_hw* chip, fptr_8nnn_test fptr_check)
{
    for (unsigned x = 0; x < REGISTER_V_COUNT; ++x)
    for (unsigned y = 0; y < REGISTER_V_COUNT; ++y)
    {
        SetVnToValues(chip, values_1);
        unsigned ret_code = fptr_check(chip, x, y);
        if (ret_code != 0)
        {
            return ret_code;
        }
    }
    return 0;
}

static int cmp_8xy0(chip8_hw* chip, unsigned x, unsigned y)
{
    unsigned expected = chip->V[ y ];
    unsigned opcode = 0x8000 | x << 8 | y << 4;
    _8xy0(chip, opcode);
    if (chip->V[ x ] != expected)
    {
        DEBUG_PRINT("Expect %u != %u\n", chip->V[ x ], expected);
        return -1;
    }
    return 0;
}

static int cmp_8xy1(chip8_hw* chip, unsigned x, unsigned y)
{
    unsigned expected = chip->V[ x ] | chip->V[ y ] ;
    unsigned opcode = 0x8001 | x << 8 | y << 4;
    _8xy1(chip, opcode);
    if (chip->V[ x ] != expected)
    {
        DEBUG_PRINT("Expect %u != %u\n", chip->V[ x ], expected);
        return -1;
    }
    return 0;
}

static int cmp_8xy2(chip8_hw* chip, unsigned x, unsigned y)
{
    unsigned expected = chip->V[ x ] & chip->V[ y ] ;
    unsigned opcode = 0x8002 | x << 8 | y << 4;
    _8xy2(chip, opcode);
    if (chip->V[ x ] != expected)
    {
        DEBUG_PRINT("Expect %u != %u\n", chip->V[ x ], expected);
        return -1;
    }
    return 0;
}

static int cmp_8xy3(chip8_hw* chip, unsigned x, unsigned y)
{
    unsigned expected = chip->V[ x ] ^ chip->V[ y ] ;
    unsigned opcode = 0x8003 | x << 8 | y << 4;
    _8xy3(chip, opcode);
    if (chip->V[ x ] != expected)
    {
        DEBUG_PRINT("Expect %u != %u\n", chip->V[ x ], expected);
        return -1;
    }
    return 0;
}

static int cmp_8xy4(chip8_hw* chip, unsigned x, unsigned y)
{
    if (x == 0xf) return 0; // V[0xf] is has a special meaning here so ignore if it is used as result register

    unsigned expected = chip->V[ x ] + chip->V[ y ];
    unsigned opcode = 0x8004 | x << 8 | y << 4;
    _8xy4(chip, opcode);
    if (chip->V[ x ] != (expected & 0xff))
    {
        DEBUG_PRINT("Expect %u != %u(%u)\n", chip->V[ x ], expected & 0xff, expected);
        return -1;
    }
    if ((expected & (~0xff)) > 0)
    {
        if (chip->V[0xf] != 1)
        {
           DEBUG_PRINT("Expect V[0xf] == 1 (%u), x:%u, y:%u\n", chip->V[ 0xf ], x, y);
           return -2;
        }
    }
    else
    {
        if (chip->V[0xf] != 0)
        {
            DEBUG_PRINT("Expect V[0xf] == 0 (%u), x:%u, y:%u\n", chip->V[ 0xf ], x, y);
            return -3;
        }
    }
    return 0;
}

static int cmp_8xy5(chip8_hw* chip, unsigned x, unsigned y)
{
    if (x == 0xf) return 0; // V[0xf] is has a special meaning here so ignore if it is used as result register

    unsigned expected = chip->V[ x ] - chip->V[ y ];
    unsigned opcode = 0x8005 | x << 8 | y << 4;
    _8xy5(chip, opcode);
    if (chip->V[ x ] != (expected & 0xff))
    {
        DEBUG_PRINT("Expect %u != %u(%u)\n", chip->V[ x ], expected & 0xff, expected);
        return -1;
    }
    if ((expected & (~0xff)) > 0)
    {
        if (chip->V[0xf] != 0)
        {
           DEBUG_PRINT("Expect V[0xf] == 0 (%u), x:%u, y:%u\n", chip->V[ 0xf ], x, y);
           return -2;
        }
    }
    else
    {
        if (chip->V[0xf] != 1)
        {
            DEBUG_PRINT("Expect V[0xf] == 1 (%u), x:%u, y:%u\n", chip->V[ 0xf ], x, y);
            return -3;
        }
    }
    return 0;
}

static int cmp_8xy6(chip8_hw* chip, unsigned x, unsigned y)
{
    if (x == 0xf) return 0; // V[0xf] is has a special meaning here so ignore if it is used as result register

    unsigned f = chip->V[ x ] & 1;
    unsigned expected = chip->V[ x ] >> 1;
    unsigned opcode = 0x8006 | x << 8 | y << 4;
    _8xy6(chip, opcode);
    if (chip->V[ x ] != (expected & 0xff))
    {
        DEBUG_PRINT("Expect %u != %u(%u)\n", chip->V[ x ], expected & 0xff, expected);
        return -1;
    }

    if (chip->V[0xf] != f)
    {
       DEBUG_PRINT("Expect V[0xf]: %u == %u, x:%u, y:%u\n", chip->V[ 0xf ], f, x, y);
       return -2;
    }
    return 0;
}

static int cmp_8xy7(chip8_hw* chip, unsigned x, unsigned y)
{
    if (x == 0xf) return 0; // V[0xf] is has a special meaning here so ignore if it is used as result register

    unsigned expected =  chip->V[ y ] - chip->V[ x ];
    unsigned opcode = 0x8007 | x << 8 | y << 4;
    _8xy7(chip, opcode);
    if (chip->V[ x ] != (expected & 0xff))
    {
        DEBUG_PRINT("Expect %u != %u(%u)\n", chip->V[ x ], expected & 0xff, expected);
        return -1;
    }
    if ((expected & (~0xff)) > 0)
    {
        if (chip->V[0xf] != 0)
        {
           DEBUG_PRINT("Expect V[0xf] == 0 (%u), x:%u, y:%u\n", chip->V[ 0xf ], x, y);
           return -2;
        }
    }
    else
    {
        if (chip->V[0xf] != 1)
        {
            DEBUG_PRINT("Expect V[0xf] == 1 (%u), x:%u, y:%u\n", chip->V[ 0xf ], x, y);
            return -3;
        }
    }
    return 0;
}

static int cmp_8xyE(chip8_hw* chip, unsigned x, unsigned y)
{
    if (x == 0xf) return 0; // V[0xf] is has a special meaning here so ignore if it is used as result register

    unsigned f = (chip->V[ x ] >> 7) & 1;
    unsigned expected = chip->V[ x ] << 1;
    unsigned opcode = 0x800e | x << 8 | y << 4;
    _8xyE(chip, opcode);
    if (chip->V[ x ] != (expected & 0xff))
    {
        DEBUG_PRINT("Expect %u != %u(%u)\n", chip->V[ x ], expected & 0xff, expected);
        return -1;
    }

    if (chip->V[0xf] != f)
    {
       DEBUG_PRINT("Expect V[0xf]: %u == %u, x:%u, y:%u\n", chip->V[ 0xf ], f, x, y);
       return -2;
    }
    return 0;
}

static int cmp_9xy0(chip8_hw* chip, unsigned x, unsigned y)
{
    unsigned expected = chip->PC;
    if (chip->V[x] != chip->V[y])
    {
        expected += 2;
    }

    unsigned opcode = 0x900e | x << 8 | y << 4;
    _9xy0(chip, opcode);
    if (chip->PC != expected)
    {
        DEBUG_PRINT("Expect PC: %u != %u, x:%u, y:%u\n", chip->PC, expected, x, y);
        return -1;
    }
    return 0;
}

// Wrappers
int test_8xy0(chip8_hw* chip)
{
    return test_8xxx(chip, cmp_8xy0);
}

int test_8xy1(chip8_hw* chip)
{
    return test_8xxx(chip, cmp_8xy1);
}

int test_8xy2(chip8_hw* chip)
{
    return test_8xxx(chip, cmp_8xy2);
}

int test_8xy3(chip8_hw* chip)
{
    return test_8xxx(chip, cmp_8xy3);
}

int test_8xy4(chip8_hw* chip)
{
    return test_8xxx(chip, cmp_8xy4);
}

int test_8xy5(chip8_hw* chip)
{
    return test_8xxx(chip, cmp_8xy5);
}

int test_8xy6(chip8_hw* chip)
{
    return test_8xxx(chip, cmp_8xy6);
}

int test_8xy7(chip8_hw* chip)
{
    return test_8xxx(chip, cmp_8xy7);
}

int test_8xyE(chip8_hw* chip)
{
    return test_8xxx(chip, cmp_8xyE);
}

int test_9xy0(chip8_hw* chip)
{
    return test_8xxx(chip, cmp_9xy0);
}
