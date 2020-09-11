#include <stdio.h>
#include <string.h>

#include "chip8.h"

bool Chip8Init( chip8_hw* chip );
void Chip8Free( chip8_hw* chip );

bool Chip8LoadProgram( const char* file );

int main( int argc, char** argv )
{

}

bool Chip8Init( chip8_hw* chip );
{
    memset( chip->v, 0, REGISTER_V_COUNT );
    memset( chip->i, 0, REGISTER_I_LEN );

    chip8->ram = (unsigned char*)calloc( CHIP8_RAM_LEN, sizeof(unsigned char) )
    if ( chip8->ram == NULL )
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
