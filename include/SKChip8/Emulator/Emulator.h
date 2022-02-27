#ifndef _EMULATOR_H
#define _EMULATOR_H

#include <Core/CPU.h>
#include <chrono>
#include <string>

namespace SKChip8
{
    using EmulatorClock = std::chrono::steady_clock;
    using EmulatorDuration = std::chrono::nanoseconds;

    static constexpr double EMULATOR_DEFAULT_FREQUENCY_MHZ = 1.79;
    static constexpr int64_t HZ_PER_MHZ = 1'000;
    static constexpr double NS_PER_S = 1'000'000'000;

    class Emulator
    {

    public:
        // ns = (1 / [MHZ] * [HZ/MHZ]) * [NS/S]
        Emulator(double clockFreq_ = EMULATOR_DEFAULT_FREQUENCY_MHZ)
            : clockPeriodNS_(NS_PER_S / (clockFreq_ * HZ_PER_MHZ)), chip8CPU_(){};

        void LoadProgram(const std::string &rompath);

        void Update();

        CPU::FrameBuffer GetFrameBuffer() const { return chip8CPU_.GetFrameBuffer(); }

        void SetKeyState(uint8_t key, bool state);

    protected:
        EmulatorDuration getElapsedTime() const;

    private:
        CPU chip8CPU_;
        double clockPeriodNS_;
        std::chrono::time_point<EmulatorClock> lastExecution_;
    };
}

#endif
