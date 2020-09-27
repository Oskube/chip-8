#include <stdlib.h>

#include "chip8.h"
#include "decoder.h"
#include "opcodes.h"

typedef struct {
    unsigned mask;
    unsigned base;
} opcode_info;

static opcode_info* opcode_table = NULL;
static unsigned     opcode_table_len = 0;

void FreeOpcodeTable()
{
    if (opcode_table) free(opcode_table);
}

void GenerateOpcodeTable()
{
    if (opcode_table != NULL) return;

    unsigned list_size = GetMnemonicCount();
    opcode_table = (opcode_info*)malloc(sizeof(opcode_info)*list_size);
    if (!opcode_table) return;
    opcode_table_len = list_size;

    atexit(FreeOpcodeTable); // Free opcode table at exit

    for (unsigned i = 0; i < list_size; i++)
    {
        const mnemonic* m = &mnemonic_list[ i ];
        opcode_table[i].base = m->base;
        opcode_table[i].mask = 0;
        for (unsigned j = 0; m->operands[ j ].fmt != NULL && j < MAX_OPERANDS; j++)
        {
            opcode_table[i].mask |= m->operands[ j ].mask;
        }
        opcode_table[i].mask = ~opcode_table[i].mask;
    }
}

unsigned DecodeOpcode(unsigned opcode)
{
    if (!opcode_table) GenerateOpcodeTable();

    for (unsigned i=0; i < opcode_table_len; i++)
    {
        opcode_info* op = opcode_table+i;
        if (op->base == (opcode & op->mask)) return i;
    }

    return INVALID_OPCODE;
}
