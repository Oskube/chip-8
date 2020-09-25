#include <stdio.h>
#include <string.h>

#include "opcodes.h"
#include "util.h"

static unsigned GetShiftFromMask(unsigned mask);
static unsigned short EncodeInstruction(char* instruction);

/* TODO: Hexadecimal numbers */


int main(int argc, char** argv)
{
    /*
    char code[] =
        //"CLS\n"
        //"RET\n"
        //"JMP 123\n"
        //"MOV [I]     V123\n"
        "SE   V1, 12";

    */
    unsigned char* code = NULL;
    unsigned code_len = 0;
    if (!ReadFile("./priv_assembler_test", false, &code, &code_len ))
    {
        return -1;
    }


    char* unprocessed = code;
    while (1)
    {
        char* next = strchr(unprocessed, '\n');
        if (next) *next = '\0';
        EncodeInstruction(unprocessed);
        if (next) unprocessed = next+1;
        else break;
    }

    return 0;
}

unsigned short EncodeInstruction(char* instruction)
{
    printf("ENCODE: %s\t", instruction);

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

    if (arg_count == 0) return 0;

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
        //printf("'%s', '%s'\n", arg[0], current_op->mnemonic);
        if (strcmp(arg[0], current_op->mnemonic) != 0 ||
            GetOperandCount(current_op) != arg_count-1)
        {
            continue;
        }
        //printf("m: %s\t%s\n", current_op->mnemonic, arg[0]);

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

            //printf("%s: %s\n", oper, current_fmt);
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
        printf("match; output: %#.4x\n", modified);
        return modified;
    }
    else
    {
        printf("NO MATCH!!\n");
    }

    return 0;
}

