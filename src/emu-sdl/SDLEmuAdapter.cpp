#include "SDLEmuAdapter.h"

void SDLEmuAdapter::Initialize()
{
    Emulator_.LoadProgram(ROMPath_);
}

std::vector<SDL_Point> SDLEmuAdapter::Draw()
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