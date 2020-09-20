#include "chip8.h"
#include "decoder.h"
#include "opcodes.h"

/* Decoder functions */
static instr_fptr _0nnn_dec(unsigned);
static instr_fptr _8XYn_dec(unsigned);
static instr_fptr _EAnn_dec(unsigned);
static instr_fptr _FXnn_dec(unsigned);

enum {
    OP_OPCODE_PTR,
    OP_DECODER_FUN,
};

typedef struct {
    unsigned type;
    union {
        instr_fptr  fun;
        instr_fptr  (*dec)(unsigned);
    };
} opcode_dec;

#define OP_FN(fn)      { OP_OPCODE_PTR , .fun=fn }
#define OP_DECODER(fn) { OP_DECODER_FUN, .dec=fn }

const opcode_dec _1st_nibble[] = {
    OP_DECODER(_0nnn_dec),
    OP_FN(_1NNN),
    OP_FN(_2NNN),
    OP_FN(_3XNN),
    OP_FN(_4XNN),
    OP_FN(_5XY0),
    OP_FN(_6XNN),
    OP_FN(_7XNN),
    OP_DECODER(_8XYn_dec),
    OP_FN(_9XY0),
    OP_FN(_ANNN),
    OP_FN(_BNNN),
    OP_FN(_CXNN),
    OP_FN(_DXYN),
    OP_DECODER(_EAnn_dec),
    OP_DECODER(_FXnn_dec),
};

instr_fptr DecodeOpcode( unsigned opcode )
{
    unsigned char nibble = (opcode >> 24) & 0xf;
    const opcode_dec* op = &_1st_nibble[ nibble ];
    if ( op->type == OP_OPCODE_PTR )
    {
        return op->fun;
    }
    else
    {
        return op->dec( opcode );
    }
}

instr_fptr _0nnn_dec( unsigned opcode )
{
    if (GET_NIBBLE(opcode, 1) == 0xe)
    {
        if (GET_NIBBLE(opcode, 0) == 0xe)
        {
            return _00EE;
        }
        return _00E0;
    }
    return _0nnn;
}

instr_fptr _8XYn_dec( unsigned opcode )
{

}

instr_fptr _EAnn_dec( unsigned opcode )
{

}

instr_fptr _FXnn_dec( unsigned opcode )
{

}

