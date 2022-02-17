#ifndef _ROMLoader_H
#define _ROMLoader_H

#include <string>
#include <cstdint>
#include <vector>

namespace SKChip8 {

static constexpr uint16_t ROM_START_ADDRESS = 0x200;

class ROMLoader
{
public:
    ROMLoader(std::string filename);

    void parse();

    std::string getDisassembly() const;
    std::string getDump() const;

private:
    std::string filename_;
    std::vector<uint16_t> buffer_;
};

}
#endif