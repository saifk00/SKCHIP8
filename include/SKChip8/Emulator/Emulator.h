#ifndef _EMULATOR_H
#define _EMULATOR_H

#include <Core/CPU.h>
#include <chrono>
#include <string>

using namespace std::chrono_literals;

namespace SKChip8
{
    using EmulatorClock = std::chrono::steady_clock;
    using EmulatorDuration = std::chrono::nanoseconds;

    // the rate at which instructions are executed. note this is not the same
    // as the clock speed which (on the COSMAC VIP) is 1.76MHz
    static constexpr auto EMULATOR_CPU_HZ = 550;

    class Emulator
    {

    public:
        Emulator()
        {
            chip8CPU_ = std::make_shared<CPU>();
            instructionsSinceLastTick_ = 0;
            instructionsPerTick_ = EMULATOR_CPU_HZ / SKChip8::TIMER_HZ;
        }

        void LoadProgram(const std::string &rompath);
        void Step();
        void SetKeyState(uint8_t key, bool state);

        CPU::FrameBuffer GetFrameBuffer() const { return chip8CPU_->GetFrameBuffer(); }
        std::shared_ptr<const CPU> GetCPU() const { return chip8CPU_; }

        uint64_t GetIPT() const { return instructionsPerTick_; }

    private:
        std::shared_ptr<CPU> chip8CPU_;
        uint64_t instructionsPerTick_;
        uint64_t instructionsSinceLastTick_;
    };
}

#endif
