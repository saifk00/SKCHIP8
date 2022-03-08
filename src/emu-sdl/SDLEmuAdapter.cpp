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
    std::vector<SDL_Point> points;
    auto arr = SKChip8::Emulator::GetFrameBuffer();
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

void SDLEmuAdapter::UpdateKeyState()
{
    const auto keyState = SDL_GetKeyboardState(NULL);

    // transform the keymap from SDL to Chip8
    for (auto &kv : KEYMAP)
    {
        SetKeyState(kv.second, keyState[kv.first]);
    }
}

void SDLEmuAdapter::SetFPS(double fps)
{
    fps_ = fps;
    instructionsPerFrame_ = SKChip8::EMULATOR_CPU_HZ / fps;
}

void SDLEmuAdapter::Enable()
{
    running_ = true;
}

void SDLEmuAdapter::Disable()
{
    running_ = false;
}

void SDLEmuAdapter::Update()
{
    if (running_)
    {
        for (int i = 0; i < instructionsPerFrame_; ++i)
        {
            SKChip8::Emulator::Step();

            if (chip8CPU_->GetSoundTimer() > 0)
            {
                tonePlayer_.Play();
            }
            else if (chip8CPU_->GetSoundTimer() == 0)
            {
                tonePlayer_.Pause();
            }
        }
    }
}