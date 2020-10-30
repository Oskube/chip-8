#include "chip8.h"

void RlInitializeWindow(float scale, const char* title);
bool RlShouldQuit();
void RlDrawScreen(chip8_hw* hw);
bool RlIsKeyDown(unsigned key);
unsigned RlGetKeyBlocking();
void RlClose();
