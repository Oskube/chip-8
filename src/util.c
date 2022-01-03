#include <stdio.h>
#include <stdlib.h>

#include "util.h"

bool ReadFile(const char* file, bool isBinary, unsigned char** output, unsigned* outlen)
{
    FILE* f = NULL;

    if (isBinary) f = fopen( file, "rb");
    else          f = fopen( file, "r" );

    if (!f) return false;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    unsigned char* out = (unsigned char*)malloc( size +1 ); // Add 1 to make sure read non-binary content is null terminated
    if (!out) return false;

    size_t read = fread(out, sizeof(unsigned char), size, f);
    fclose( f );
    if (read != size)
    {
        free(out);
        return false;
    }

    if (!isBinary) out[size] = '\0'; // If text file make sure it is NULL terminater

    *output = out;
    *outlen = read;

    return true;
}

bool WriteFile(const char* file, bool isBinary, unsigned char* data, unsigned len)
{
    FILE* f = NULL;

    if (isBinary) f = fopen( file, "wb");
    else          f = fopen( file, "w" );

    if (!f) return false;

    size_t bytes = fwrite(data, 1, len, f);
    fclose(f);
    if (bytes != len) return false;

    return true;
}
