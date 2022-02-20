#include <Utils/ROMLoader.h>
#include <Utils/CHIP8ISA.h>

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

        while (!ifs.eof())
        {
            char c[2];
            ifs.read(c, sizeof(uint16_t));

            // data is in big-endian format, so swap the bytes after
            data.push_back((static_cast<uint16_t>(c[0]) << 8) | c[1]);
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
        instr->dump(disassembly) << '\n';

        addr += 2;
    }

    return disassembly.str();
}

std::vector<uint16_t> ROMLoader::getROM() const
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
