#include "Emulator.h"

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

    void Emulator::Reset()
    {
        chip8CPU_ = std::make_shared<CPU>();
        instructionsSinceLastTick_ = 0;
        reloadROM();
    }

    void Emulator::reloadROM()
    {
        chip8CPU_->LoadROM(ROM_.getROM());
    }

    void Emulator::LoadProgram(const std::string &rompath)
    {
        ROM_ = ROMLoader(rompath);

        reloadROM();
    }

    void Emulator::SetKeyState(uint8_t key, bool state)
    {
        chip8CPU_->SetKeyState(key, state);
    }

}
