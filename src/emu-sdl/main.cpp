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

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    auto rom = argc < 2 ? "../roms/maze.ch8" : argv[1];

    {
        auto emulator = std::make_shared<SDLEmuAdapter>(rom);
        emulator->Initialize();

        DebuggingWindow debugWindow(emulator);
        EmulatorWindow emulatorWindow(emulator);

        bool shouldStop = false;
        while (!shouldStop)
        {
            emulator->UpdateKeyState(SDL_GetKeyboardState(NULL));

            // close if either window gets closed
            // TODO(sk00) do this only if the main emulator window is closed
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_WINDOWEVENT &&
                    event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    shouldStop = true;
                }
            }

            emulatorWindow.Update();
            debugWindow.Update();

            SDL_Delay(10);
        }

        debugWindow.Destroy();
        emulatorWindow.Destroy();
    }

    SDL_Quit();
    return 0;
}