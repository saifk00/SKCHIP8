#ifndef _CHIP8_CPU_H_
#define _CHIP8_CPU_H_

#include "Utils/CHIP8ISA.h"

#include <memory>
#include <cstdint>
#include <vector>

namespace SKChip8
{
    static constexpr size_t REG_COUNT = 16;
    static constexpr uint16_t CHIP8_REG_BITS = 12;
    static constexpr uint16_t CHIP8_MEM_SIZE = 1 << 12;
    static constexpr uint16_t PROG_MEMORY_OFFSET = 0x200;

    class CPU
    {
    public:
        CPU();

        // loads a program ROM into the code region
        void LoadROM(std::vector<uint8_t> buffer);

        // updates state by one cycle
        void DoCycle();

    protected:
        uint8_t &currentInstruction();
        void handleInstruction(Instruction &inst);
        void handleInstruction(MachineInstruction &inst);
        void handleInstruction(ALUInstruction &inst);
        void handleInstruction(KeyInstruction &inst);
        void handleInstruction(ControlInstruction &inst);

    private:
        // big endian memory
        std::array<uint8_t, CHIP8_MEM_SIZE> memory_;
        std::array<uint8_t, REG_COUNT> registerFile_;
        // actually only 12 bits due to the memory capacity of chip-8
        uint16_t indexRegister_;
        uint64_t systemClock_;
        uint16_t programCounter_;
    };
}
#endif