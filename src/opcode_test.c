#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opcodes.h"
#include "chip8.h"

#define DEBUG_PRINT( fmt, ... )  fprintf(stderr, "\t\t%s(...): " fmt, __FUNCTION__,__VA_ARGS__)
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

static int test_Annn(chip8_hw*);
static int test_Bnnn(chip8_hw*);
static int test_Cxnn(chip8_hw*);
static int test_Dxyn(chip8_hw*);
static int test_Ex9E(chip8_hw*);
static int test_ExA1(chip8_hw*);
static int test_Fx07(chip8_hw*);
static int test_Fx0A(chip8_hw*);
static int test_Fx15(chip8_hw*);
static int test_Fx18(chip8_hw*);
static int test_Fx1E(chip8_hw*);
static int test_Fx29(chip8_hw*);
static int test_Fx33(chip8_hw*);
static int test_Fx55_Fx65(chip8_hw*);

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

    { test_8xy0, "Opcode 8xy0" }, /* Test wrappers using same int test_8xxx(...) */
    { test_8xy1, "Opcode 8xy1" },
    { test_8xy2, "Opcode 8xy2" },
    { test_8xy3, "Opcode 8xy3" },
    { test_8xy4, "Opcode 8xy4" },
    { test_8xy5, "Opcode 8xy5" },
    { test_8xy6, "Opcode 8xy6" },
    { test_8xy7, "Opcode 8xy7" },
    { test_8xyE, "Opcode 8xyE" },
    { test_9xy0, "Opcode 9xy0" },

    { test_Annn, "Opcode Annn" },
    { test_Bnnn, "Opcode Bnnn" },
    { test_Cxnn, "Opcode Cxnn" },
    { test_Dxyn, "Opcode Dxyn" },
    { test_Ex9E, "Opcode Ex9E" },
    { test_ExA1, "Opcode ExA1" },
    { test_Fx07, "Opcode Fx07" },
    { test_Fx0A, "Opcode Fx0A" },
    { test_Fx15, "Opcode Fx15" },
    { test_Fx18, "Opcode Fx18" },
    { test_Fx1E, "Opcode Fx1E" },
    { test_Fx29, "Opcode Fx29" },
    { test_Fx33, "Opcode Fx33" },
    { test_Fx55_Fx65, "Opcode Fx55 & Fx65" },

    { NULL, NULL },
};

static const unsigned char values_0[ REGISTER_V_COUNT ] = { 0 };
static const unsigned char values_1[ REGISTER_V_COUNT ] = {
    0x00, 0x22, 0x33, 0x44,
    0x55, 0x66, 0x77, 0x88,
    0x99, 0xaa, 0xbb, 0xcc,
    0xdd, 0xee, 0xef, 0xff
};

static const unsigned char values_ordered[] = {
    0  , 1  , 2  , 3,
    4  , 5  , 6  , 7,
    8  , 9  , 0xa, 0xb,
    0xc, 0xd, 0xe, 0xf
};

// Functions for keyboard testing
static bool key_is_down = false;
static bool is_key_down(unsigned key)
{
    return key_is_down;
}
static unsigned key_down_value = 0;
static unsigned get_key_blocking()
{
    return key_down_value;
}

int main()
{
    chip8_hw chip;
    for (unsigned cur_test = 0; 1; ++cur_test)
    {
        const test_entry* cur = &(tests[ cur_test ]);
        if (cur->test_fun == NULL) break;

        Chip8Init( &chip );
        chip.is_key_down      = is_key_down;
        chip.get_key_blocking = get_key_blocking;

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
        _4xnn(chip, opcode);
    }
    if (chip->PC != REGISTER_V_COUNT * 2)
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
        DEBUG_PRINT("Expect %u != %u(%u) opcode: 0x%.2x\n", chip->V[ x ], expected & 0xff, expected, opcode);
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

    unsigned expected = chip->V[ y ] - chip->V[ x ];
    unsigned opcode = 0x8007 | x << 8 | y << 4;
    _8xy7(chip, opcode);
    if (chip->V[ x ] != (expected & 0xff))
    {
        DEBUG_PRINT("Expect %u != %u(%u) opcode: 0x%.2x\n", chip->V[ x ], expected & 0xff, expected, opcode);
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
// -- End of wrappers


int test_Annn(chip8_hw* chip)
{
    for (unsigned int i = 0; i < 0xfff; ++i)
    {
        unsigned opcode = 0xa000 | i;
        _Annn(chip, opcode);
        if (chip->I != i)
        {
            DEBUG_PRINT("Expect I: %u != %u\n", chip->I, i);
            return -1;
        }
    }
    return 0;
}

int test_Bnnn(chip8_hw* chip)
{
    for (unsigned i = 0; i < 100; ++i)
    {
        chip->V[ 0 ] = rand() & 0xff;
        unsigned value  = rand() & 0xfff;
        unsigned opcode = 0xb000 | value;
        unsigned expected = value + chip->V[ 0 ];
        _Bnnn(chip, opcode);
        if (chip->PC != expected)
        {
            DEBUG_PRINT("Expect PC: %u != %u, opcode: 0x%.4x\n", chip->PC, expected, opcode);
            return -1;
        }
    }
    return 0;
}

int test_Cxnn(chip8_hw* chip)
{
    unsigned char mask[] = { 0x0f, 0xf0, 0xff };
    for (unsigned m=0; m < 3; ++m)
    for (unsigned v=0; v < REGISTER_V_COUNT; ++v)
    {
        unsigned tries = 0;
        unsigned opcode = 0xc000 | (v << 8) | mask[ m ];
        _Cxnn(chip, opcode);
        unsigned last = chip->V[ v ];
        for (; tries < 10; ++tries)
        {
            _Cxnn(chip, opcode);
            if ((chip->V[ v ] & ~mask[ m ]) == 0)
            {
                if (last != chip->V[ v ])
                {
                    break;
                }
            }
            else
            {
                DEBUG_PRINT("Expect V[%u] 0x%.2x & 0x%.2x == 0; Mask not working, mask: 0x%.2x\n", v, chip->V[ v ], ~mask[m], mask[m]);
                return -1;
            }
        }
        if (tries == 10)
        {
            DEBUG_PRINT("10 tries without new random value, opcode: 0x%.4x\n", opcode);
            return -2;
        }
    }
    return 0;
}

int test_Fx07(chip8_hw* chip)
{
    for (unsigned v=0; v < REGISTER_V_COUNT; ++v)
    for (unsigned i=0; i <= 0xff; ++i)
    {
        chip->DT = i;
        unsigned opcode = 0xf007 | (v << 8);
        _Fx07(chip, opcode);
        if(chip->V[v] != i)
        {
            DEBUG_PRINT("Except %u != %u, opcode: 0x%.4x\n", chip->V[v], i, opcode);
            return -1;
        }
    }
    return 0;
}

int test_Fx15_Fx18(chip8_hw* chip, unsigned base, instr_fptr op_fun, unsigned char* reg)
{
    for (unsigned v=0; v < REGISTER_V_COUNT; ++v)
    for (unsigned i=0; i <= 0xff; ++i)
    {
        chip->V[v] = i;
        unsigned opcode = base | (v << 8);
        op_fun(chip, opcode);
        if(*reg != i)
        {
            DEBUG_PRINT("Except %u != %u, opcode: 0x%.4x\n", *reg, i, opcode);
            return -1;
        }
    }
    return 0;
}

int test_Fx15(chip8_hw* chip)
{
    return test_Fx15_Fx18(chip, 0xf015, _Fx15, &(chip->DT));
}

int test_Fx18(chip8_hw* chip)
{
    return test_Fx15_Fx18(chip, 0xf018, _Fx18, &(chip->ST));
}

int test_Fx1E(chip8_hw* chip)
{
    SetVnToValues(chip, values_1);
    chip->I = rand();
    for(unsigned v=0; v < REGISTER_V_COUNT; ++v)
    {
        unsigned expected = (chip->I + chip->V[ v ]) & 0xffff;
        unsigned opcode = 0xf01e | (v << 8);
        _Fx1E(chip, opcode);
        if (chip->I != expected)
        {
            DEBUG_PRINT("Except I: %u != %u, opcode: 0x%.4x\n", chip->I, expected, opcode);
            return -1;
        }
    }
    return 0;
}

int test_Fx29(chip8_hw* chip)
{
    char pos[0x10] = {};
    for (unsigned i=0; i < 0x10; ++i)
    {
        pos[ i ] = CHIP8_RAM_CHARSET_BEGIN + (i * CHIP8_CHAR_LEN);
    }

    SetVnToValues(chip, values_ordered);
    for (unsigned v=0; v < REGISTER_V_COUNT; ++v)
    {
        unsigned opcode = 0xf029 | (v << 8);
        _Fx29(chip, opcode);
        if (chip->I != pos[ v ])
        {
            DEBUG_PRINT("Except I: %u != %u, opcode: 0x%.4x\n", chip->I, pos[ v ], opcode);
            return -1;
        }
    }
    return 0;
}

int test_Fx33(chip8_hw* chip)
{
    SetVnToValues(chip, values_1);
    for (unsigned v=0; v < REGISTER_V_COUNT; ++v)
    {
        unsigned bcd[3] = {
            (chip->V[ v ] / 100) % 10,
            (chip->V[ v ] / 10 ) % 10,
             chip->V[ v ] % 10,
        };
        unsigned opcode = 0xf033 | (v << 8);
        _Fx33(chip, opcode);
        if( chip->ram[chip->I +0] != bcd[ 0 ] &&
            chip->ram[chip->I +1] != bcd[ 1 ] &&
            chip->ram[chip->I +2] != bcd[ 2 ]
        )
        {
            DEBUG_PRINT("Except RAM[I]: (%u,%u,%u) != (%u,%u,%u), opcode: 0x%.4x\n",
                chip->ram[ chip->I ], chip->ram[ chip->I +1 ], chip->ram[ chip->I +2],
                bcd[0], bcd[1], bcd[2],
                opcode);
            return -1;
        }
    }
    return 0;
}

int test_Fx55_Fx65(chip8_hw* chip)
{
    SetVnToValues(chip, values_1);
    unsigned pos[] = {
        0x100, 0x200, 0x300, 0x400,
        0x500, 0x600, 0x700, 0x800,
        0x900, 0xa00, 0xb00, 0xc00,
        0xd00, 0xe00, 0x000, 0xe80
    };
    for (unsigned v=0; v < REGISTER_V_COUNT; ++v)
    {
        chip->I = pos[ v ];
        unsigned opcode = 0xf055 | (v << 8);
        _Fx55(chip, opcode);
        for (unsigned i=0; i <= v; ++i)
        {
            if (chip->ram[ chip->I +i ] != values_1[ i ])
            {
                DEBUG_PRINT("Except RAM[%u]: %u != %u, opcode: 0x%.4x\n",
                    chip->I+i, chip->ram[ chip->I +i ], values_1[ i ],
                    opcode);
                return -1;
            }
        }
    }
    for (unsigned v=0; v < REGISTER_V_COUNT; ++v)
    {
        SetVnToValues(chip, values_0);
        chip->I = pos[ v ];
        unsigned opcode = 0xf065 | (v << 8);
        _Fx65(chip, opcode);
        for (unsigned i=0; i <= v; ++i)
        {
            if (chip->V[ i ] != values_1[ i ])
            {
                DEBUG_PRINT("Except V[%u]: %u != %u, opcode: 0x%.4x\n",
                    i, chip->V[ i ], values_1[ i ],
                    opcode);
                return -2;
            }
        }
    }
    return 0;
}

int test_Ex9E(chip8_hw* chip)
{
    SetVnToValues(chip, values_ordered);
    for (unsigned v=0; v < REGISTER_V_COUNT; ++v)
    {
        unsigned expected = chip->PC;
        unsigned opcode = 0xe09e | (v << 8);
        key_is_down = false;
        _Ex9E(chip, opcode);
        if (chip->PC != expected)
        {
            DEBUG_PRINT("Except PC: %u != %u, opcode: 0x%.4x\n",
                chip->PC, expected, opcode);
            return -1;
        }
        expected = chip->PC+2;
        key_is_down = true;
        _Ex9E(chip, opcode);
        if (chip->PC != expected)
        {
            DEBUG_PRINT("Except PC: %u != %u, opcode: 0x%.4x\n",
                chip->PC, expected, opcode);
            return -2;
        }
    }
    return 0;
}

int test_ExA1(chip8_hw* chip)
{
    SetVnToValues(chip, values_ordered);
    for (unsigned v=0; v < REGISTER_V_COUNT; ++v)
    {
        unsigned expected = chip->PC;
        unsigned opcode = 0xe0a1 | (v << 8);
        key_is_down = true;
        _ExA1(chip, opcode);
        if (chip->PC != expected)
        {
            DEBUG_PRINT("Except PC: %u != %u, opcode: 0x%.4x\n",
                chip->PC, expected, opcode);
            return -1;
        }
        expected = chip->PC+2;
        key_is_down = false;
        _ExA1(chip, opcode);
        if (chip->PC != expected)
        {
            DEBUG_PRINT("Except PC: %u != %u, opcode: 0x%.4x\n",
                chip->PC, expected, opcode);
            return -2;
        }
    }
    return 0;
}

int test_Fx0A(chip8_hw* chip)
{
    for (unsigned v=0; v<REGISTER_V_COUNT; ++v)
    {
        for (unsigned i=0; i < 0x10; ++i)
        {
            key_down_value = i;
            unsigned opcode = 0xf00a | (v << 8);
            _Fx0A(chip, opcode);
            if (chip->V[ v ] != key_down_value)
            {
                DEBUG_PRINT("Except V[%u]: %u != %u, opcode: 0x%.4x\n",
                    v, chip->V[ v ], key_down_value, opcode);
                return -1;
            }
        }
    }
    return 0;
}

int test_Dxyn(chip8_hw* chip)
{
    _Fx1E(chip, 0xf01e); // Point I to mem v[0] (which is 0);
    _Dxyn(chip, 0xd01f); // Draw sprite to x=v[0], y=v[1]; from mem[I], 0xf rows
    chip->V[0] = 4;
    _Dxyn(chip, 0xd01f);

    for (unsigned i=0; i < 0xf; i++)
    {
        unsigned gfx_pos = i * (CHIP8_GFX_W/8);
        unsigned char cmp = chip->ram[i];
        cmp = cmp | (cmp >> 4);
        if (chip->gfx[gfx_pos] != cmp)
        {
            DEBUG_PRINT("Except gfx[%u]: %u != %u",
                gfx_pos, chip->gfx[ gfx_pos ], cmp);
            return -i;
        }
    }
    if (chip->V[0xf] != 0)
    {
        DEBUG_PRINT("Except V[0xf] to be 0", "");
        return -17;
    }

    // Clear everything
    chip->V[0] = 0;
    _Fx1E(chip, 0xf01e);
    _Dxyn(chip, 0xd01f);
    chip->V[0] = 4;
    _Dxyn(chip, 0xd01f);

    for (unsigned i=0; i < 0xf; i++)
    {
        unsigned gfx_pos = i * (CHIP8_GFX_W/8);
        if (chip->gfx[gfx_pos] != 0)
        {
            DEBUG_PRINT("Except gfx[%u]: %u != %u",
                gfx_pos, chip->gfx[ gfx_pos ], 0);
            return -i;
        }
    }
    // V[0xf] should be set now
    if (chip->V[0xf] != 1)
    {
        DEBUG_PRINT("Except V[0xf] to be set", "");
        return -18;
    }

#warning  More tests should be made for this opcode

    return 0;
}
