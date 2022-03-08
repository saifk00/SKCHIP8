# CHIP-8

## Basic system info
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
- Endianness: big endian

## Timing info
- the [COSMAC VIP](https://www.wikiwand.com/en/COSMAC_VIP) ran at 1.76MHz
    - however, this ends up being an actual instruction execution rate of about 500Hz
    - the timers are 60Hz so the instruction:timer ratio is 500Hz / 60Hz ~= 8
        - so to get semi-accurate timing while also allowing the timers to be controlled by the debugger, we decrement timers every 8 instructions
- design
    - to separate concerns, we have the following hierarchy
        - CPU: when it cycles, it executes exactly one instruction and doesnt touch the timer
            - knows only about the 60Hz value
        - Emulator: when told to update, cycles the CPU once and updates the timer values every 8 instructions
            - knows about the 60Hz value through CPU and the 500Hz value
        - Emulator Adapters: for whatever output platform, tries to maintain that 500Hz value by telling the Emulator to update
            - knows about real time as well as 60Hz and 500Hz through the emulator
        - lets say we're running at 50fps
            - 500 (inst/sec) / 50 (frame/sec) = 10 (inst/frame)


NOTE:
make sure to place SDL2.dll and mingw's libstdc++-6.dll in the same folder as the exe if using that
also point to the SDL2 install dir in FindSDL2.cmake if its in a different location than SDL2_SEARCH_PATHS