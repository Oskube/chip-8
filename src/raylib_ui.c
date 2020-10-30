#include <assert.h>
#include "raylib.h"

#include "raylib_ui.h"

#define MAX_KEY 0x10

static float win_scale = 1;
static int keymap[ MAX_KEY ] = {
    KEY_ONE,
    KEY_TWO,
    KEY_THREE,
    KEY_FOUR,

    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_R,

    KEY_A,
    KEY_S,
    KEY_D,
    KEY_F,

    KEY_Z,
    KEY_X,
    KEY_C,
    KEY_V,
};

void RlInitializeWindow(float scale, const char* title)
{
    win_scale = scale;
    InitWindow(CHIP8_GFX_W*scale, CHIP8_GFX_H*scale, title);
    SetTargetFPS(60);
}

bool RlShouldQuit()
{
    return WindowShouldClose();
}

void RlDrawScreen(chip8_hw* hw)
{
    BeginDrawing();
        ClearBackground(SKYBLUE);

        for (unsigned i = 0; i < CHIP8_GFX_LEN; i++)
        {
            unsigned char byte = hw->gfx[i];

            unsigned y = i * 8 / CHIP8_GFX_W,
                     x = i * 8 % CHIP8_GFX_W;
            for (unsigned char bit = 0x80; bit > 0; bit >>= 1, x++)
            {
                if (byte & bit)
                {
                    DrawRectangle(x * win_scale, y * win_scale,
                                  1 * win_scale, 1 * win_scale,
                                  RED);
                }
            }
        }
    EndDrawing();
}

bool RlIsKeyDown(unsigned key)
{
    assert(key > 0x10);
    return IsKeyDown(keymap[key]);
}

unsigned RlGetKeyBlocking()
{
    while (1)
    {
        int key = GetKeyPressed();
        unsigned i = 0;
        for (; i < MAX_KEY; i++)
        {
            if (keymap[i] == key) break;
        }
        if (i == MAX_KEY) continue;
        if (IsKeyDown(key))
        {
            return key;
        }
    }
}

void RlClose()
{
    CloseWindow();
}
