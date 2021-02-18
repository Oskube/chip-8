#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "raylib.h"

#include "raylib_ui.h"

#define MAX_KEY 0x10

static bool  quit = false;
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

#define MAX_SAMPLES  512
#define MAX_SAMPLES_PER_UPDATE  4096
#define AUDIO_TONE_FREQ  220
#define AUDIO_VOLUME     3000
static struct {
    AudioStream stream;
    short* wave;
    short* buffer;
    unsigned wave_length;
    unsigned last_position;
} audio;


void RlInitializeWindow(float scale, const char* title)
{
    win_scale = scale;
    InitWindow(CHIP8_GFX_W*scale, CHIP8_GFX_H*scale, title);
    SetTargetFPS(60);

    InitAudioDevice();
    audio.stream = InitAudioStream(22050, 16, 1);

    PlayAudioStream(audio.stream);
    audio.wave   = (short*)malloc(sizeof(short) * MAX_SAMPLES);
    audio.buffer = (short*)malloc(sizeof(short) * MAX_SAMPLES_PER_UPDATE);
    audio.last_position = 0;

    audio.wave_length = 22050/AUDIO_TONE_FREQ;
    for (unsigned i = 0; i < audio.wave_length * 2; i++)
    {
        audio.wave[i] = (short)(sinf(2*PI*((float)i)/audio.wave_length) * AUDIO_VOLUME);
    }
}

bool RlShouldQuit()
{
    // Poll events
    BeginDrawing();
    EndDrawing();

    if (WindowShouldClose()) quit = true;
    return quit;
}

void RlDrawScreen(chip8_hw* hw)
{
    if (hw->ST > 0)
    {
        RlAudioPlay();
    }

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
    while (!RlShouldQuit())
    {
        for (unsigned i=0; i < MAX_KEY; i++)
        {
            if (IsKeyDown(keymap[i]))
            {
                return i;
            }
        }
    }
    return 0;
}

void RlClose()
{
    CloseAudioStream(audio.stream);
    free(audio.wave);
    free(audio.buffer);
    CloseAudioDevice();
    CloseWindow();
}

void RlAudioPlay()
{
    if (IsAudioStreamProcessed(audio.stream))
    {
        unsigned offset = audio.last_position;
        unsigned w_len = audio.wave_length - offset;
        if (w_len == 0)  w_len = audio.wave_length;

        for (unsigned pos = 0; pos < MAX_SAMPLES_PER_UPDATE; offset = 0, pos += w_len)
        {
            if (pos > 0)
            {
                w_len = audio.wave_length;
            }
            if ((pos+w_len) > MAX_SAMPLES_PER_UPDATE)
            {
                w_len = MAX_SAMPLES_PER_UPDATE - pos;
            }

            memcpy(audio.buffer + pos,
                   audio.wave + offset,
                   w_len * sizeof(short));
        }
        audio.last_position = w_len;

        UpdateAudioStream(audio.stream, audio.buffer, MAX_SAMPLES_PER_UPDATE);
    }
}
