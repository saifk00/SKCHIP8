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
}

void ROMLoader::parse()
{
    std::ifstream ifs(filename_, std::ios::binary | std::ios::in);

    if (!ifs.is_open())
    {
        std::cout << "failed to open ROM file " << filename_ << '\n';
        throw new std::exception();
    }
    else
    {
        std::vector<uint16_t> data;

        while (1)
        {
            char c[2];
            ifs.read(c, sizeof(uint16_t));
	    if (ifs.eof()) break;
 
            // data is in big-endian format, so swap the bytes after
            data.push_back((static_cast<uint8_t>(c[0]) << 8) | static_cast<uint8_t>(c[1]));
        }

        this->buffer_ = data;
    }

    ifs.close();
}

std::string ROMLoader::getDisassembly() const
{
    std::stringstream disassembly;
    auto addr = ROM_START_ADDRESS;

    for (const auto &raw_instr : buffer_)
    {
        auto instr = SKChip8::DecodeInstruction(raw_instr);
        disassembly << "0x" << std::hex << std::setfill('0') << std::setw(4) << addr << ": ";
	try {
        instr->dump(disassembly) << '\n';
	} catch (const std::exception& e) {
	  disassembly << "DW\t0x" << std::hex << std::setfill('0') << std::setw(4) << raw_instr << '\n';
	}
        addr += 2;
    }

    return disassembly.str();
}

std::vector<uint8_t> ROMLoader::getROM() const
{
    std::vector<uint8_t> result;
    result.reserve(buffer_.size() * sizeof(uint16_t) / sizeof(uint8_t));
    for (const auto &word : buffer_)
    {
        result.emplace_back(word & 0xFF);
        result.emplace_back((word >> 8) & 0xFF);
    }

    return result;
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
