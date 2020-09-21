#include "chip8.h"
#include "decoder.h"
#include "opcodes.h"

/* Decoder functions */
static instr_fptr _0nnn_dec(unsigned);
static instr_fptr _8XYn_dec(unsigned);
static instr_fptr _EXnn_dec(unsigned);
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
    OP_FN(_1nnn),
    OP_FN(_2nnn),
    OP_FN(_3xnn),
    OP_FN(_4xnn),
    OP_FN(_5xy0),
    OP_FN(_6xnn),
    OP_FN(_7xnn),
    OP_DECODER(_8XYn_dec),
    OP_FN(_9xy0),
    OP_FN(_Annn),
    OP_FN(_Bnnn),
    OP_FN(_Cxnn),
    OP_FN(_Dxyn),
    OP_DECODER(_EXnn_dec),
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

const instr_fptr _8XYn_funs[0x10] = {
    _8xy0,
    _8xy1,
    _8xy2,
    _8xy3,
    _8xy4,
    _8xy5,
    _8xy6,
    _8xy7,
    _invalid_op,
    _invalid_op,
    _invalid_op,
    _invalid_op,
    _invalid_op,
    _invalid_op,
    _8xyE,
    _invalid_op
};

instr_fptr _8XYn_dec( unsigned opcode )
{
    unsigned nib = GET_NIBBLE(opcode, 0);
    return _8XYn_funs[ nib ];
}

instr_fptr _EXnn_dec( unsigned opcode )
{
    unsigned char cmp = opcode & 0xff;
    if (cmp == 0x9e)
    {
        return _Ex9E;
    }
    else if (cmp == 0xa1)
    {
        return _ExA1;
    }
    return _invalid_op;
}

instr_fptr _FXnn_dec( unsigned opcode )
{
    unsigned char cmp = opcode & 0xff;

    switch (cmp)
    {
        case 0x07: return _Fx07;
        case 0x0A: return _Fx0A;
        case 0x15: return _Fx15;
        case 0x18: return _Fx18;
        case 0x1E: return _Fx1E;
        case 0x29: return _Fx29;
        case 0x33: return _Fx33;
        case 0x55: return _Fx55;
        case 0x65: return _Fx65;

        default: return _invalid_op;
    }
}

