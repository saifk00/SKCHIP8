#ifndef _CHIP8ISA_H
#define _CHIP8ISA_H

#include <string>
#include <cstdint>

namespace SKChip8
{
    std::string DecodeInstruction(uint16_t opcode);
}

#endif