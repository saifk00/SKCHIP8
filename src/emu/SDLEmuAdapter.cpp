#include "SDLEmuAdapter.h"

void SDLEmuAdapter::Initialize()
{
    Emulator_.LoadProgram(ROMPath_);
}

void SDLEmuAdapter::Update()
{
    Emulator_.Update();
}

std::vector<SDL_Point> SDLEmuAdapter::Draw()
{
    std::vector<SDL_Point> points;
    auto arr = Emulator_.GetFrameBuffer();
    for (int i = 0; i < arr.size(); ++i)
    {
        for (int j = 0; j < arr[i].size(); ++j)
        {
            if (arr[i][j])
            {
                points.push_back(SDL_Point{i, j});
            }
        }
    }

    return points;
}