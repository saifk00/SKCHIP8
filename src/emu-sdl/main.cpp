#include <SDL.h>

#include "SDLEmuAdapter.h"

#include <SKChip8/Core/CPU.h>
#include <SKChip8/Utils/ROMLoader.h>

#include <iostream>

#include "glad/glad.h"

#include "DebuggingWindow.hpp"
#include "EmulatorWindow.hpp"

int main(int argc, char *argv[])
{
    SDL_Event event;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    auto rom = argc < 2 ? "../roms/maze.ch8" : argv[1];
    {
        auto emulator = std::make_shared<SDLEmuAdapter>(rom);

        DebuggingWindow debugWindow(emulator);
        EmulatorWindow emulatorWindow(emulator);

        bool shouldStop = false;
        while (!shouldStop)
        {
            // handle and dispatch events
            while (SDL_PollEvent(&event))
            {
                // TODO(sk00) do this better
                if (event.window.windowID == debugWindow.GetWindowID())
                {
                    debugWindow.HandleEvent(event);
                }
                else if (event.window.windowID == emulatorWindow.GetWindowID())
                {
                    emulatorWindow.HandleEvent(event);
                }
                else if (event.type == SDL_QUIT)
                {
                    shouldStop = true;
                    break;
                }
            }

            emulatorWindow.Update();
            debugWindow.Update();
        }
    }

    SDL_Quit();
    return 0;
}