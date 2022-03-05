#include <SKChip8/Emulator/Emulator.h>

#include <iostream>
#include <string>

static constexpr char RETURN_TO_TOP[] = "\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r";

int main(int argc, char *argv[])
{
    auto rom = argc < 2 ? "../roms/maze.ch8" : argv[1];
    SKChip8::Emulator emulator;
    emulator.LoadProgram(rom);

    bool shouldStop = false;
    while (!shouldStop)
    {
        emulator.Step();
        auto frame = emulator.GetFrameBuffer();

        for (size_t i = 0; i < frame.size(); ++i)
        {
            for (size_t j = 0; j < frame[i].size(); ++j)
            {
                if (frame[i][j])
                {
                    std::cout << "*";
                }
                else
                {
                    std::cout << " ";
                }
            }
            std::cout << std::endl;
        }

        // return to the top
        std::cout << RETURN_TO_TOP;
    }
}