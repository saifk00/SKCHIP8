#include "Emulator.h"

#include <Utils/ROMLoader.h>

namespace SKChip8
{
    void Emulator::Update()
    {
        if (!running_)
            return;

        int64_t cycles = getElapsedTime().count() / clockPeriodNS_;

        chip8CPU_.Cycle();

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
        r.parse();

        chip8CPU_.LoadROM(r.getROM());

        lastExecution_ = EmulatorClock::now();
    }

    EmulatorDuration Emulator::getElapsedTime() const
    {
        auto now = EmulatorClock::now();
        return (now - lastExecution_);
    }

    void Emulator::SetKeyState(uint8_t key, bool state)
    {
        chip8CPU_.SetKeyState(key, state);
    }

    void Emulator::Start()
    {
        running_ = true;
    }

    void Emulator::Stop()
    {
        running_ = false;
    }

    void Emulator::Step()
    {
        chip8CPU_.StartTimer();
        chip8CPU_.Cycle();
        chip8CPU_.StopTimer();
    }
}
