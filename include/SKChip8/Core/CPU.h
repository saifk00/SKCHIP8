#ifndef _CHIP8_CPU_H_
#define _CHIP8_CPU_H_

#include <Utils/CHIP8ISA.h>

#include <memory>
#include <cstdint>
#include <vector>
#include <stack>

namespace SKChip8
{
    static constexpr size_t REG_COUNT = 16;
    static constexpr uint16_t CHIP8_REG_BITS = 12;
    static constexpr uint16_t CHIP8_MEM_SIZE = 1 << 12;
    static constexpr uint16_t PROG_MEMORY_OFFSET = 0x200;
    static constexpr uint8_t SCR_HEIGHT = 32;
    static constexpr uint8_t SCR_WIDTH = 64;
    static constexpr size_t KEY_COUNT = 16;
    static constexpr uint16_t FONT_MEMORY_OFFSET = 0x000;
    // each font sprite is 5 bytes
    static constexpr uint16_t FONT_BYTES = 5;

    class CPU
    {
    public:
        CPU();

        // loads a program ROM into the code region
        void LoadROM(std::vector<uint8_t> buffer);

        // updates state by one cycle
        void Cycle();

        using FrameBuffer = std::array<std::array<uint8_t, SCR_WIDTH>, SCR_HEIGHT>;
        FrameBuffer GetFrameBuffer() const;

    protected:
        uint16_t currentInstruction();
        void handleInstruction(Instruction &inst);
        void handleInstruction(MachineInstruction &inst);
        void handleInstruction(ALUInstruction &inst);
        void handleInstruction(KeyInstruction &inst);
        void handleInstruction(ControlInstruction &inst);

        void drawSprite(uint8_t x, uint8_t y, uint8_t n);

        void timerTick();

    private:
        // true if the keyboard changed state since the last cycle
        bool isKeyboardDirty() const { return externState_ & KEYBOARD_DIRTY_BIT; }

        constexpr size_t flattenedFrameBufferIndex(uint8_t x, uint8_t y) const { return (SCR_WIDTH >> 4) * y + (x >> 8); }

        // big endian memory
        std::array<uint8_t, CHIP8_MEM_SIZE> memory_;
        std::array<uint8_t, REG_COUNT> registerFile_;
        // actually only 12 bits due to the memory capacity of chip-8
        uint16_t indexRegister_;
        uint64_t systemClock_;
        uint16_t programCounter_;

        // framebuffer in a row-major packed format
        // pixel (x, y) occurs at A[(SCR_WIDTH/8)*y + (x/8)] >> (8 - (x % 8))
        std::array<uint8_t, (SCR_WIDTH / 8) * SCR_HEIGHT> frameBuffer_;
        std::stack<uint16_t> callStack_;

        // TODO(sk00) implement this from the injection
        std::array<bool, KEY_COUNT> keyState_;

        // whether the CPU is awaiting IO, etc.
        bool halted_ = false;

        // if halted, the register that should be written with the value
        // of the pressed key
        uint8_t registerAwaitingKey_;

        uint8_t delayTimer_;
        uint8_t soundTimer_;

        // state updated async to cpu clock cycles (e.g. keyboard)
        uint8_t externState_;
        static constexpr uint8_t KEYBOARD_DIRTY_BIT = 0;
    };
}
#endif
