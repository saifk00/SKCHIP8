# CHIP-8

- 4KiB memory (12 bit addrs)
- Word size = 8 bits
- Memory map
    - 0x000 - 0x1FF : Chip 8 interpreter (contains font set in emu)
    - 0x200         : program starts here
    - 0xEA0 - 0xEFF : call stack, internal use, variables
    - 0xF00 - 0xFFF : display refresh
- 16 8-bit registers V0-F
- 12-bit index register I
- Timers
    - 60Hz, down count and stop at 0
    - Delay: can be set/read
    - Sound: make a beep when nonzero
- Input
    - 0xF keyboard
    - 8, 4, 6, 2 typically for motion, maybe remap these to the typical vim keys
- Display
    - 64x32 pixels, monochrome
    - (1-15)x8 pixel sprites get xord


NOTE:
make sure to place SDL2.dll and mingw's libstdc++-6.dll in the same folder as the exe if using that
also point to the SDL2 install dir in FindSDL2.cmake if its in a different location than SDL2_SEARCH_PATHS