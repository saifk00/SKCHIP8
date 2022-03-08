#ifndef _ROMLoader_H
#define _ROMLoader_H

#include <string>
#include <cstdint>
#include <vector>

namespace SKChip8
{

    static constexpr uint16_t ROM_START_ADDRESS = 0x200;

    class ROMLoader
    {
    public:
        ROMLoader() = default;
        ROMLoader(std::string filename);

        std::string getDisassembly() const;
        std::string getDump() const;
        std::vector<uint8_t> getROM() const;

    private:
        std::string filename_;
        std::vector<uint8_t> buffer_;
    };

}
#endif