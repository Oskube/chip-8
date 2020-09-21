#include <stdio.h>
#include <string.h>

#include "chip8.h"

bool Chip8Init( chip8_hw* chip );
void Chip8Free( chip8_hw* chip );
bool Chip8LoadProgram( const char* file );
bool Chip8Dump( chip8_hw* chip, FILE* output );

int main( int argc, char** argv )
{

}

bool Chip8Init( chip8_hw* chip );
{
    memset( chip->V    , 0, REGISTER_V_COUNT );
    memset( chip->stack, 0, CHIP8_STACK_LEN  );
    memset( chip->gfx  , 0, CHPI8_GFX_LEN    );

    chip->I  = 0;
    chip->PC = 0;
    chip->DT = 0;
    chip->ST = 0;
    chip->stack_top = 0;

    chip->get_key = NULL;

    chip->ram = (unsigned char*)calloc( CHIP8_RAM_LEN, sizeof(unsigned char) )
    if ( chip->ram == NULL )
    {
        return false;
    }

    return true;
}

void Chip8Free( chip8_hw* chip  )
{
    if ( chip )
    {
        free(chip->ram)
        chip->ram = NULL;
    }
}

bool Chip8LoadProgram( const char* file, chip8_hw* chip )
{
    if ( chip->ram == NULL ) return false;

    FILE* prog = fopen( file, "rb" );
    if ( !prog ) return false;

    size_t bytes = fread(chip->ram[ CHIP8_PROG_START ], sizeof(unsigned char), CHIP8_PROG_MAX_LEN, prog );
    fclose( prog );

    if ( bytes == 0 ) return false;

    return true;
}
