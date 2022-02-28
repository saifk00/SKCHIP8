#include "SDLEmuAdapter.h"

#include <map>

static constexpr std::pair<SDL_Scancode, uint8_t> KEYMAP[SKChip8::KEY_COUNT] = {
    {SDL_SCANCODE_1, 0x1},
    {SDL_SCANCODE_2, 0x2},
    {SDL_SCANCODE_3, 0x3},
    {SDL_SCANCODE_4, 0xC},
    {SDL_SCANCODE_Q, 0x4},
    {SDL_SCANCODE_W, 0x5},
    {SDL_SCANCODE_E, 0x6},
    {SDL_SCANCODE_R, 0xD},
    {SDL_SCANCODE_A, 0x7},
    {SDL_SCANCODE_S, 0x8},
    {SDL_SCANCODE_D, 0x9},
    {SDL_SCANCODE_F, 0xE},
    {SDL_SCANCODE_Z, 0xA},
    {SDL_SCANCODE_X, 0x0},
    {SDL_SCANCODE_C, 0xB},
    {SDL_SCANCODE_V, 0xF}};

std::vector<SDL_Point> SDLEmuAdapter::GetFrameBuffer()
{
    Emulator_.Update();

    std::vector<SDL_Point> points;
    auto arr = Emulator_.GetFrameBuffer();
    for (int y = 0; y < arr.size(); ++y)
    {
        for (int x = 0; x < arr[y].size(); ++x)
        {
            if (arr[y][x])
            {
                points.push_back(SDL_Point{x, y});
            }
        }
    }

    return points;
}

void SDLEmuAdapter::UpdateKeyState(const uint8_t *keyState)
{
    // transform the keymap from SDL to Chip8
    for (auto &kv : KEYMAP)
    {
        Emulator_.SetKeyState(kv.second, keyState[kv.first]);
    }
}

void SDLEmuAdapter::Stop()
{
    Emulator_.Stop();
}

void SDLEmuAdapter::Start()
{
    Emulator_.Start();
}

void SDLEmuAdapter::Step()
{
    Emulator_.Step();
}