#ifndef _CHIP8_UTILS_H_
#define _CHIP8_UTILS_H_

#include <iostream>
#include <iomanip>
#include <string>

namespace SKChip8
{
    struct HexPrinter
    {
        HexPrinter(uint16_t val, uint8_t bits) : Bits(bits), Value(val){};

        friend std::ostream &operator<<(std::ostream &os, const HexPrinter &printer)
        {
            return os << std::hex << std::setfill('0') << std::setw(printer.Bits / 4) << printer.Value;
        }

        uint16_t Value;
        uint8_t Bits;
    };
}

#endif