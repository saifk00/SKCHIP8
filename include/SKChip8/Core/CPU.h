#ifndef _CHIP8_CPU_H_
#define _CHIP8_CPU_H_

#include "Utils/CHIP8ISA.h"

#include <memory>
#include <cstdint>
#include <vector>
#include <stack>
#include <chrono>
#include <mutex>
#include <thread>

using namespace std::chrono_literals;

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
    static constexpr uint16_t FRAME_BUFFER_SIZE = (SCR_WIDTH / 8) * SCR_HEIGHT;

    // timer period (60Hz)
    static constexpr auto TIMER_PERIOD = 16.67ms;
    static constexpr auto TIMER_HZ = 60.0;

    class CPU
    {
    public:
        CPU();

        // loads a program ROM into the code region
        void LoadROM(std::vector<uint8_t> buffer);

        // updates state by one cycle
        void Cycle();

        // set the key state
        void SetKeyState(uint8_t key, bool state);

        using FrameBuffer = std::array<std::array<bool, SCR_WIDTH>, SCR_HEIGHT>;
        FrameBuffer GetFrameBuffer() const;

        std::string DumpState() const;

        // Updates the timers by one tick
        void TimerTick();

        uint16_t GetPC() const { return programCounter_; }
        uint16_t GetCurrentInstruction() const { return currentInstruction(); }
        std::array<uint8_t, CHIP8_MEM_SIZE> GetMemory() const { return memory_; }
        std::array<uint8_t, REG_COUNT> GetRegisters() const { return registerFile_; }
        std::array<bool, KEY_COUNT> GetKeyState() const { return keyState_; }
        std::pair<uint8_t, uint8_t> GetTimers() const
        {
            return std::make_pair(delayTimer_, soundTimer_);
        }
        uint16_t GetIndexPointer() const { return indexRegister_; }

    protected:
        uint16_t currentInstruction() const;
        void handleInstruction(Instruction &inst);
        void handleInstruction(MachineInstruction &inst);
        void handleInstruction(ALUInstruction &inst);
        void handleInstruction(KeyInstruction &inst);
        void handleInstruction(ControlInstruction &inst);

        void drawSprite(uint8_t x, uint8_t y, uint8_t n);

        std::string dumpSpecial() const;
        std::string dumpRegisters() const;
        std::string dumpMemory() const;
        std::string dumpFrameBuffer() const;
        std::string dumpStack() const;
        std::string dumpKeyboard() const;

    private:
        // true if the keyboard changed state since the last cycle
        bool isKeyboardDirty() const { return externState_ & KEYBOARD_DIRTY_BIT; }

        constexpr size_t flattenedFrameBufferIndex(uint8_t x, uint8_t y) const { return (SCR_WIDTH / 8) * y + (x / 8); }

        // big endian memory
        std::array<uint8_t, CHIP8_MEM_SIZE> memory_;
        std::array<uint8_t, REG_COUNT> registerFile_;
        // actually only 12 bits due to the memory capacity of chip-8
        uint16_t indexRegister_;
        uint64_t systemClock_;
        uint16_t programCounter_;
        bool shouldIncrementPC_;

        // framebuffer in a row-major packed format
        // pixel (x, y) occurs at A[(SCR_WIDTH/8)*y + (x/8)] >> (8 - (x % 8))
        std::array<uint8_t, FRAME_BUFFER_SIZE> frameBuffer_;
        std::stack<uint16_t> callStack_;

        std::array<bool, KEY_COUNT> keyState_;

        // whether the CPU is awaiting IO, etc.
        bool halted_ = false;

        // if halted, the register that should be written with the value
        // of the pressed key
        uint8_t registerAwaitingKey_;

        // timer values
        uint8_t delayTimer_;
        uint8_t soundTimer_;

        // state updated async to cpu clock cycles (e.g. keyboard)
        uint8_t externState_;
        static constexpr uint8_t KEYBOARD_DIRTY_BIT = 0;
    };
}
#endif
