#ifndef SDL_EMU_ADAPTER_H
#define SDL_EMU_ADAPTER_H

#include "SDL.h"
#include <string>
#include <SKChip8/Emulator/Emulator.h>

class SDLEmuAdapter
{
public:
    SDLEmuAdapter(const std::string &rompath) : ROMPath_(rompath), Emulator_() {}

    void Initialize();
    void Update();
    std::vector<SDL_Point> Draw();

private:
    SKChip8::Emulator Emulator_;
    std::string ROMPath_;
};

#endif