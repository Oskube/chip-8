#include <stdbool.h>

bool ReadFile(const char* file, bool isBinary, unsigned char** output, unsigned* len);
bool WriteFile(const char* file, bool isBinary, unsigned char* data, unsigned len);
