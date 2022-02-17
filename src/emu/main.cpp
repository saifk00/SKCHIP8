#include <SKChip8/CPU.h>
#include <SKChip8/Utils/ROMLoader.h>

#include <SDL.h>

#include <iostream>

int main(int argc, char const *argv[])
{
    SDL_Window *window = nullptr;
    SDL_Surface *surface = nullptr;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if ((window = SDL_CreateWindow("SK-CHIP8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 320, SDL_WINDOW_SHOWN)) == nullptr)
    {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Delay(500);

    // TODO(sk00) remove this
    auto rom = SKChip8::ROMLoader("../roms/maze.ch8");
    rom.parse();

    std::cout << rom.getDisassembly() << std::endl;
    std::cout << rom.getDump() << std::endl;

    SDL_Quit();
    return 0;
}
