#include "Emulator.h"

#include <Utils/ROMLoader.h>

namespace SKChip8
{
    void Emulator::Update()
    {
        int64_t cycles = getElapsedTime().count() / clockPeriodNS_;
        for (size_t i = 0; i < cycles; ++i)
        {
            chip8CPU_.Cycle();
        }

        // when it was called too soon, wait and perform a few cycles
        // at once next time
        if (cycles > 0)
        {
            lastExecution_ = EmulatorClock::now();
        }
    }

    void Emulator::LoadProgram(const std::string &rompath)
    {
        ROMLoader r(rompath);

        chip8CPU_.LoadROM(r.getROM());

        lastExecution_ = EmulatorClock::now();
    }

    EmulatorDuration Emulator::getElapsedTime() const
    {
        auto now = EmulatorClock::now();
        return (now - lastExecution_);
    }
}