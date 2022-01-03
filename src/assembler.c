#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <endian.h>

#include "decoder.h"
#include "opcodes.h"
#include "token.h"
#include "util.h"


static bool AppendString(char** buffer, unsigned* buffer_len, const char* new_str);
static unsigned GetShiftFromMask(unsigned mask);
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
