#include "CPU.h"

namespace SKChip8
{
    CPU::CPU()
    {
        // set up a memory region
        // initialize registers
        // initialize timers, peripherals, etc.
        memory_ = std::make_shared<Memory>(CHIP8_MEM_SIZE, CHIP8_REG_BITS);
    }
}