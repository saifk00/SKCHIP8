#include "Memory.h"

uint8_t &Memory::operator[](uint16_t addr)
{
    return memory_.at(addr & addrMask_);
}