#ifndef SDL_EMU_ADAPTER_H
#define SDL_EMU_ADAPTER_H

#include "SDL.h"

#include <string>
#include <SKChip8/Emulator/Emulator.h>

class SDLEmuAdapter : public SKChip8::Emulator
{
public:
    SDLEmuAdapter(const std::string &rompath) : ROMPath_(rompath)
    {
        LoadProgram(ROMPath_);
        running_ = false;
        SetFPS(60);
    }

    std::vector<SDL_Point> GetFrameBuffer();
    void UpdateKeyState();
    void Enable();
    void Disable();
    void Update();
    void SetFPS(double fps);

    double GetFPS() const { return fps_; }
    double GetInstructionsPerFrame() const { return instructionsPerFrame_; }

private:
    std::string ROMPath_;
    bool running_;
    uint64_t instructionsPerFrame_;
    double fps_;
};

#endif