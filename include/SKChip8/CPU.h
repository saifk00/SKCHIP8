#ifndef _CHIP8_CPU_H_
#define _CHIP8_CPU_H_

#include <memory>
#include <cstdint>

#include "Memory.h"

namespace SKChip8
{
    static constexpr size_t REG_COUNT = 16;
    static constexpr uint16_t CHIP8_REG_BITS = 12;
    static constexpr uint16_t CHIP8_MEM_SIZE = 1 << 12;

    class CPU
    {
    public:
        // TODO(sk00) dependency inject a frame and audio buffer to connect this to display,
        // audio, input, and timers
        CPU();

        // loads a program ROM into the code region
        void LoadROM(std::vector<uint16_t> buffer);

    private:
        // shared pointer to allow debugging, etc.
        std::shared_ptr<Memory> memory_;
        std::array<uint8_t, REG_COUNT> registerFile_;
        // actually only 12 bits due to the memory capacity of chip-8
        uint16_t indexRegister_;
    };
}
#endif