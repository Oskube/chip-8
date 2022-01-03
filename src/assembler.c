#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <endian.h>

#include "decoder.h"
#include "opcodes.h"
#include "token.h"
#include "util.h"

/* TODO: Allow hexadecimal numbers in source files */

static bool AppendString(char** buffer, unsigned* buffer_len, const char* new_str);
static unsigned GetShiftFromMask(unsigned mask);
static bool EncodeInstruction(char* instruction, unsigned short* opcode);
static bool DecodeInstruction(unsigned short instr, char* output);

static unsigned Assemble(const char* source, const char* out_file);
static unsigned Disassemble(const char* file, const char* out_file);

#define MAX_PROG_LEN 1024

#define MAX_INSTR_CHR 32
#define BUF_CHUNK_SZ 512

int main(int argc, char** argv)
{
    static const char* help_str =
        "USAGE ./assembler <INPUT> <OUTPUT> [options]\n"
        "\n"
        "Options:\n"
        "  -d  Disassemble input file\n";

    if (argc < 3 || argc > 4)
    {
        fprintf(stderr, "%s", help_str);
        return -1;
    }

    const char* input  = argv[1];
    const char* output = argv[2];
    bool disassemble = false;

    if (argc == 4)
    {
        if (strcmp(argv[3], "-d") == 0)
        {
            disassemble = true;
        }
        else
        {
            fprintf(stderr, "%s", help_str);
            return -1;
        }
    }

    if (!input  ||
        !output ||
        input[0]  == '-' || /* File names cannot begin with '-' */
        output[0] == '-')
    {
        fprintf(stderr, "Input file or ouput file paths missing!\n%s", help_str);
        return -1;
    }

    if (disassemble)
    {
        return Disassemble(input, output);
    }
    else
    {
        return Assemble(input, output);
    }
}

unsigned Assemble(const char* source, const char* out_file)
{
    unsigned char* code = NULL;
    unsigned code_len = 0;
    if (!ReadFile(source, false, &code, &code_len ))
    {
        fprintf(stderr, "Failed to read source file\n");
        return -1;
    }

    unsigned char* output = NULL;
    unsigned output_len = 0;
    CompileSource(code, &output, &output_len);
    free(code);

    if(!output)
    {
        fprintf(stderr, "Failed to succesfully compile source\n");
        return -2;
    }

    unsigned status = 0;
    if(!WriteFile(out_file, true, (unsigned char*)output, output_len))
    {
        fprintf(stderr, "Failed to write output file\n");
        status = -3;
    }
    free(output);

    return status;
}

unsigned Disassemble(const char* file, const char* out_file)
{
    unsigned char* bin = NULL;
    unsigned bin_len = 0;
    if (!ReadFile(file, true, &bin, &bin_len ))
    {
        fprintf(stderr, "Failed to read binary file\n");
        return -1;
    }

    char* output_str = NULL;
    unsigned output_len = 0;
    char decoded_str[ MAX_INSTR_CHR ] = {0};
    for (unsigned i = 0; i < bin_len; i += 2)
    {
        unsigned short* cur = (unsigned short*)&bin[i];
        if (DecodeInstruction( be16toh(*cur), decoded_str ))
        {
            AppendString(&output_str, &output_len, decoded_str);
        }
        else
        {
            fprintf(stderr, "Failed to decode instruction '%.4x'\n", *cur);
        }
    }
    free(bin);

    if(!WriteFile(out_file, false, (unsigned char*)output_str, output_len))
    {
        fprintf(stderr, "Failed to write output file\n");
        return -3;
    }
    free(output_str);

    return 0;
}

bool DecodeInstruction(unsigned short instr, char* output)
{
    unsigned index = DecodeOpcode( instr );
    if (index == INVALID_OPCODE)
    {
        return false;
    }

    const char* mnemonic_str = mnemonic_list[ index ].mnemonic;
    const operand* opers     = mnemonic_list[ index ].operands;

    strcpy(output, mnemonic_str);

    for (unsigned i=0; i < 3 && opers[i].fmt; i++)
    {
        unsigned l = strlen(output);
        if (i > 0)
        {
            output[l++] = ',';
        }
        output[l++] = ' ';
        output[l]   = '\0';

        strcpy( output + l, opers[i].fmt);
        if (opers[i].mask != 0)
        {
            unsigned value = GetValueByMask(instr, opers[i].mask);
            char fmt_str[128];
            strcpy(fmt_str, output);
            sprintf( output, fmt_str, value);
        }
    }
    return true;
}

bool AppendString(char** buffer, unsigned* buffer_len, const char* new_str)
{
    unsigned buf_chunks = (*buffer_len / BUF_CHUNK_SZ) + 1;
    unsigned new_len    = strlen(new_str) + *buffer_len +2;

    // Reallocate more space if needed
    if (new_len > buf_chunks * BUF_CHUNK_SZ ||
        *buffer == NULL)
    {
        buf_chunks = new_len / BUF_CHUNK_SZ +1;
        *buffer = (char*)realloc(*buffer, buf_chunks * BUF_CHUNK_SZ);
    }
    if (*buffer == NULL) return false;

    strcpy( *buffer + *buffer_len, new_str);
    (*buffer)[new_len-2]   = '\n';
    (*buffer)[new_len-1] = '\0';
    *buffer_len = new_len-1;
    return true;
}

bool EncodeInstruction(char* instruction, unsigned short* opcode)
{
    /*
        Go through every character replacing whitespace with '\0'
    */
    #define MAX_ARG 4
    unsigned arg_count = 0;
    char*    arg[ MAX_ARG ] = {0};
    char last = '\0';
    for (char* i=instruction; *i != '\0'; i++)
    {
        if (*i == ' ')
        {
            *i = '\0';
        }
        else if (*i == ';') /* Ignore comments */
        {
            *i = '\0'; // If comment immediately after last arg
            break;
        }
        else if (last == '\0')
        {
            arg[arg_count] = i;
            arg_count++;
            if (arg_count >= MAX_ARG) break;
        }
        last = *i;
    }

    if (arg_count == 0) return false;

    /* Check mnemonic and arg count matches
       Then compare arguments and find the correct one
       e.g. If arg has '%' use sscanf otherwise strmcp
    */
    const mnemonic* current_op = mnemonic_list;
    bool match = false;
    unsigned modified = 0, mask = 0; /* Applied operands and their ORed bit-mask */
    for (;
         current_op->fun != NULL;
         current_op++)
    {
        // Find matching mnemonic
        if (strcmp(arg[0], current_op->mnemonic) != 0 ||
            GetOperandCount(current_op) != arg_count-1)
        {
            continue;
        }
        //printf("%s MATCHES %s\n", arg[0], current_op->mnemonic);

        match = true;
        modified = current_op->base;
        mask = 0;
        /* Process operands */
        for (unsigned oper_num = 1; oper_num < arg_count; oper_num++)
        {
            char* oper = arg[oper_num];
            const char* current_fmt = current_op->operands[oper_num-1].fmt;

            // Ignore ',' from operands except last
            if (oper_num != (arg_count-1))
            {
                unsigned len = strlen(oper);
                if (len > 0 && oper[len-1] == ',')
                {
                    oper[len-1] = '\0';
                }
            }

            if (strchr(current_fmt, '%')) /* Has variable */
            {
                unsigned value = 0;
                if (sscanf(oper, current_fmt, &value) == 0)
                {
                    match = false;
                    break;
                }
                unsigned cur_mask = current_op->operands[oper_num-1].mask;
                modified |= ApplyMaskToValue(cur_mask, value);
                mask |= cur_mask;
            } /* String constant e.g. register name */
            else if (strcmp(current_fmt, oper) != 0)
            {
                match = false;
            }
        }
        if (match) break;
    }

    if (match)
    {
        *opcode = modified;
        return true;
    }

    return false;
}

