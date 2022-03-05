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
    - the timers are 60Hz so the instruction:timer ratio is 1.76MHz / 60Hz = 29,333
        - so to get semi-accurate timing while also allowing the timers to be controlled by the debugger, we decrement timers every 29,333 instructions
        - note that the period of the CPU is 1/(1.76e6) = 568.2ns, which is probably much faster than we can make the call to Cycle()
            - however, if we simply cycled `ceil(ns_since_last_call / 568.2)` times, then we would eventually fall out of sync as the fractional cycles add up
- design
    - to separate concerns, we have the following hierarchy
        - CPU: when it cycles, it executes exactly one instruction and doesnt touch the timer
            - knows only about the 60Hz value
        - Emulator: when told to update, cycles the CPU once and updates the timer values every 29,333 such times
            - knows about the 60Hz value through CPU and the 1.76MHz value
        - Emulator Adapters: for whatever output platform, tries to maintain that 1.76MHz value by telling the Emulator to update
            - knows about real time as well as 60Hz and 1.76MHz through the emulator
        - lets say we run at 60fps
            - 1.79MHz = 1,790,000 cycles per second
            - 1,790,000 (cyc/sec) / 60 (frame/sec) = 298833 cycles per frame

- actually, it turns out that while the CPU ran at 1.79MHz, it actually only computed about 500 instructions per second

TODO(sk00) update the above explanation for the new timing method


NOTE:
make sure to place SDL2.dll and mingw's libstdc++-6.dll in the same folder as the exe if using that
also point to the SDL2 install dir in FindSDL2.cmake if its in a different location than SDL2_SEARCH_PATHS