#ifndef _CHIP8_MEM_H_
#define _CHIP8_MEM_H_

#include <cstdint>
#include <vector>

class Memory
{
public:
    Memory(uint16_t bytes, uint16_t addrBits) : memory_(bytes, 0), addrMask_((1 << addrBits) - 1){};

    uint8_t &operator[](uint16_t addr);

private:
    std::vector<uint8_t> memory_;
    const uint16_t addrMask_;
};

#endif