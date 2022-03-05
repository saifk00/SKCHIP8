#include "ROMLoader.h"
#include "CHIP8ISA.h"

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <ios>

using namespace SKChip8;

ROMLoader::ROMLoader(std::string filename)
{
    this->filename_ = filename;

    std::ifstream ifs(filename_, std::ios::in | std::ios::binary);

    if (!ifs.is_open())
    {
        throw std::runtime_error("Could not open file: " + filename_);
    }

    this->buffer_ = std::vector<uint8_t>(
        std::istreambuf_iterator<char>(ifs),
        std::istreambuf_iterator<char>());
}

std::string ROMLoader::getDisassembly() const
{
    std::stringstream disassembly;
    auto addr = ROM_START_ADDRESS;

    std::vector<uint16_t> instructions;
    for (size_t i = 0; i < buffer_.size() / 2; ++i)
    {
        instructions.push_back(buffer_[i * 2] << 8 | buffer_[i * 2 + 1]);
    }

    for (const auto &raw_instr : instructions)
    {
        auto instr = SKChip8::DecodeInstruction(raw_instr);
        disassembly << "0x" << std::hex << std::setfill('0') << std::setw(4) << addr << ": ";
        try
        {
            instr->dump(disassembly) << '\n';
        }
        catch (const std::exception &e)
        {
            disassembly << "DW\t0x" << std::hex << std::setfill('0') << std::setw(4) << (((raw_instr >> 8) | raw_instr << 8) & 0xFFFF) << '\n';
        }
        addr += 2;
    }

    return disassembly.str();
}

std::vector<uint8_t> ROMLoader::getROM() const
{
    return buffer_;
}

std::string ROMLoader::getDump() const
{
    std::stringstream dump;

    for (const auto &instr : buffer_)
    {
        dump << "0x" << std::hex << std::setfill('0') << std::setw(4) << instr << '\n';
    }

    return dump.str();
}
