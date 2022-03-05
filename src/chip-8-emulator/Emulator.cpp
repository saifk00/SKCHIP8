#include "Emulator.h"

#include <Utils/ROMLoader.h>

namespace SKChip8
{
    void Emulator::Step()
    {
        chip8CPU_->Cycle();
        instructionsSinceLastTick_++;
        if (instructionsSinceLastTick_ >= instructionsPerTick_)
        {
            instructionsSinceLastTick_ = 0;
            chip8CPU_->TimerTick();
        }
    }

    void Emulator::LoadProgram(const std::string &rompath)
    {
        ROMLoader r(rompath);

        chip8CPU_->LoadROM(r.getROM());
    }

    void Emulator::SetKeyState(uint8_t key, bool state)
    {
        chip8CPU_->SetKeyState(key, state);
    }

}
