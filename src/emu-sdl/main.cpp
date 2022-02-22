#include <SDL.h>

#include "SDLEmuAdapter.h"

#include <SKChip8/Core/CPU.h>
#include <SKChip8/Utils/ROMLoader.h>

#include <iostream>

static constexpr int SCR_WIDTH = 640;
static constexpr int SCR_HEIGHT = 320;

int main(int argc, char *argv[])
{
    SDL_Surface *surface = nullptr;
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Event event;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (SDL_CreateWindowAndRenderer(640, 320, SDL_WINDOW_RESIZABLE, &window, &renderer))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        return 3;
    }

    SDL_RenderSetLogicalSize(renderer, SKChip8::SCR_WIDTH, SKChip8::SCR_HEIGHT);

    auto rom = argc < 2 ? "../roms/maze.ch8" : argv[1];
    SDLEmuAdapter emulator(rom);
    emulator.Initialize();

    bool shouldStop = false;
    while (!shouldStop)
    {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT)
        {
            shouldStop = true;
        }

        auto frame = emulator.Draw();

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderDrawPoints(renderer, frame.data(), frame.size());
        SDL_RenderPresent(renderer);

        SDL_Delay(10);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
