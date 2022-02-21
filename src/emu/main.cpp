#include "SDLEmuAdapter.h"

#include <SKChip8/Core/CPU.h>
#include <SKChip8/Utils/ROMLoader.h>

#include <SDL.h>

#include <iostream>

static constexpr int SCR_WIDTH = 640;
static constexpr int SCR_HEIGHT = 320;

int main(int argc, char const *argv[])
{
    SDL_Surface *surface = nullptr;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    auto window = SDL_CreateWindow("SK-CHIP8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCR_WIDTH, SCR_HEIGHT, SDL_WINDOW_SHOWN);
    auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::cerr << SDL_GetError() << "\n";
        return 1;
    }

    SDL_RenderSetLogicalSize(renderer, SKChip8::SCR_WIDTH, SKChip8::SCR_HEIGHT);

    SDLEmuAdapter emulator("../roms/maze.ch8");
    emulator.Initialize();

    bool shouldStop = false;
    while (!shouldStop)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                shouldStop = true;
            }
        }

        SDL_Delay(10);

        emulator.Update();
        auto frame = emulator.Draw();

        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderDrawPoints(renderer, frame.data(), frame.size());
        SDL_RenderDrawPoint(renderer, SCR_HEIGHT / 2, SCR_WIDTH / 2);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
