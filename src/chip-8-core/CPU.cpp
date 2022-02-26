#include "CPU.h"

#include <Utils/CHIP8Utils.h>

#include <cstdlib>
#include <ctime>
#include <cstring>
#include <algorithm>

static constexpr std::array<uint16_t, 256> BCD_Table = {
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x40,
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x60,
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x80,
    0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x100,
    0x101, 0x102, 0x103, 0x104, 0x105, 0x106, 0x107, 0x108, 0x109, 0x110, 0x111, 0x112, 0x113, 0x114, 0x115, 0x116, 0x117, 0x118, 0x119, 0x120,
    0x121, 0x122, 0x123, 0x124, 0x125, 0x126, 0x127, 0x128, 0x129, 0x130, 0x131, 0x132, 0x133, 0x134, 0x135, 0x136, 0x137, 0x138, 0x139, 0x140,
    0x141, 0x142, 0x143, 0x144, 0x145, 0x146, 0x147, 0x148, 0x149, 0x150, 0x151, 0x152, 0x153, 0x154, 0x155, 0x156, 0x157, 0x158, 0x159, 0x160,
    0x161, 0x162, 0x163, 0x164, 0x165, 0x166, 0x167, 0x168, 0x169, 0x170, 0x171, 0x172, 0x173, 0x174, 0x175, 0x176, 0x177, 0x178, 0x179, 0x180,
    0x181, 0x182, 0x183, 0x184, 0x185, 0x186, 0x187, 0x188, 0x189, 0x190, 0x191, 0x192, 0x193, 0x194, 0x195, 0x196, 0x197, 0x198, 0x199, 0x200,
    0x201, 0x202, 0x203, 0x204, 0x205, 0x206, 0x207, 0x208, 0x209, 0x210, 0x211, 0x212, 0x213, 0x214, 0x215, 0x216, 0x217, 0x218, 0x219, 0x220,
    0x221, 0x222, 0x223, 0x224, 0x225, 0x226, 0x227, 0x228, 0x229, 0x230, 0x231, 0x232, 0x233, 0x234, 0x235, 0x236, 0x237, 0x238, 0x239, 0x240,
    0x241, 0x242, 0x243, 0x244, 0x245, 0x246, 0x247, 0x248, 0x249, 0x250, 0x251, 0x252, 0x253, 0x254};

static constexpr uint8_t font_data[SKChip8::FONT_DATA_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

namespace
{
    struct BytePrinter
    {
        SKChip8::HexPrinter _printer;
        BytePrinter(uint8_t val) : _printer(val, 8) {}

        friend std::ostream &operator<<(std::ostream &os, const BytePrinter &reg)
        {
            return os << "0x" << reg._printer;
        }
    };

    struct PlainBytePrinter
    {
        SKChip8::HexPrinter _printer;
        PlainBytePrinter(uint8_t val) : _printer(val, 8) {}

        friend std::ostream &operator<<(std::ostream &os, const PlainBytePrinter &reg)
        {
            return os << reg._printer;
        }
    };

    struct WordPrinter
    {
        SKChip8::HexPrinter _printer;
        WordPrinter(uint16_t val) : _printer(val, 16) {}

        friend std::ostream &operator<<(std::ostream &os, const WordPrinter &reg)
        {
            return os << "0x" << reg._printer;
        }
    };
}

namespace SKChip8
{
    // TODO(sk00) dependency inject a frame and audio buffer to connect this to display,
    // audio, input, and timers
    CPU::CPU()
    {
        // initialize timers, peripherals, etc.
        std::srand(std::time(0));

        // store the font data in memory, and set the rest to zero just to be safe
        std::memset(memory_.data(), 0, memory_.size());
        std::memcpy(memory_.data() + FONT_MEMORY_OFFSET, font_data, FONT_DATA_SIZE);

        // initialize registers, keyboard, and clear the framebuffer by default
        // some programs will manually clear it at the start but others do not
        std::memset(registerFile_.data(), 0, registerFile_.size());
        std::memset(keyState_.data(), 0, keyState_.size());
        std::memset(frameBuffer_.data(), 0, frameBuffer_.size());

        delayTimer_ = 0;
        soundTimer_ = 0;
        systemClock_ = 0;

        // start a real-time thread for the 60Hz timers
        timerThread_ = std::thread([](CPU *cpu)
                                   {
            while(true) {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
                cpu->timerTick();
            } },
                                   this);
    }

    CPU::~CPU()
    {
        timerThread_.join();
    }

    void CPU::LoadROM(std::vector<uint8_t> buffer)
    {
        std::copy(buffer.begin(), buffer.end(), memory_.begin() + PROG_MEMORY_OFFSET);
        programCounter_ = PROG_MEMORY_OFFSET;
        shouldIncrementPC_ = true;
    }

    uint16_t CPU::currentInstruction()
    {
        return memory_[programCounter_] << 8 | memory_[programCounter_ + 1];
    }

    void CPU::drawSprite(uint8_t x, uint8_t y, uint8_t n)
    {
        const uint8_t offset = x % 8;
        for (size_t idx = 0; idx < n; ++idx)
        {
            const auto &row = memory_[indexRegister_ + idx];
            const auto frameIndex = flattenedFrameBufferIndex(x, y + idx);
            // since the framebuffer is stored 8 bits at a time, we need
            // to perform some masking to get the data correct.

            // offset tells us the position of the first bit from the left
            // that should get XORed (i.e. in a framebuffer byte)

            // example: offset = 3
            // framebuffer: 8b'11011001  8'b01101110
            // sprite row: 8'b11000011
            // x = 12, offset = 12 % 8 = 4
            //                 11011001 01101110
            //                 --->1100 0011
            //                 =================
            //                 11010101 01011110

            // Also, Vf should be set to 1 if any pixels were flipped from set to unset.
            // since sprites are XORed into the framebuffer, this happens when
            // the framebuffer and sprite are both a 1 in the same bit which is equivalent
            // to a bitwise AND.

            // we don't need to mask the framebuffer when performing this AND because the
            // types are unsigned so this is a logical shift and therefore the missing bits
            // will be zeros
            const auto spriteThisPart = (row >> offset);
            const auto spriteNextPart = (row << (8 - offset));

            // always set the current byte
            registerFile_[VF_] = (frameBuffer_[frameIndex] & spriteThisPart)
                                     ? 1
                                     : 0;
            frameBuffer_[frameIndex] ^= spriteThisPart;

            // the sprite may overflow to the next byte
            if (offset > 0 && frameIndex < FRAME_BUFFER_SIZE - 1)
            {
                registerFile_[VF_] = (frameBuffer_[frameIndex + 1] & spriteNextPart)
                                         ? 1
                                         : registerFile_[VF_];
                frameBuffer_[frameIndex + 1] ^= spriteNextPart;
            }
        }
    }

    void CPU::handleInstruction(Instruction &inst)
    {
        using InstructionType = Instruction::InstructionType;
        switch (inst.Type)
        {
        case InstructionType::Goto:
            // TODO(sk00) do this better. this assumes that the pc is incremented after the instruction
            // is handled so it sets it to one before the next instruction to execute
            programCounter_ = inst.Address();
            shouldIncrementPC_ = false;
            break;
        case InstructionType::Call:
            callStack_.push(programCounter_);
            programCounter_ = inst.Address();
            shouldIncrementPC_ = false;
            break;
        case InstructionType::SkipIfEqual:
            if (registerFile_[inst.RegisterX()] == inst.Immediate())
            {
                programCounter_ += 4;
                shouldIncrementPC_ = false;
            }
            break;
        case InstructionType::SkipIfNotEqual:
            if (registerFile_[inst.RegisterX()] != inst.Immediate())
            {
                programCounter_ += 4;
                shouldIncrementPC_ = false;
            }
            break;
        case InstructionType::SkipIfRegistersEqual:
            if (registerFile_[inst.RegisterX()] == registerFile_[inst.RegisterY()])
            {
                programCounter_ += 4;
                shouldIncrementPC_ = false;
            }
            break;
        case InstructionType::MoveRegisterXImmediate:
            registerFile_[inst.RegisterX()] = inst.Immediate();
            break;
        case InstructionType::AddRegisterImmediate:
            registerFile_[inst.RegisterX()] += inst.Immediate();
            break;
        case InstructionType::SkipIfNotEqualRegisters:
            if (registerFile_[inst.RegisterX()] != registerFile_[inst.RegisterY()])
            {
                programCounter_ += 4;
                shouldIncrementPC_ = false;
            }
            break;
        case InstructionType::SetAddressImmediate:
            indexRegister_ = inst.Address();
            break;
        case InstructionType::JumpLong:
            programCounter_ = registerFile_[V0_] + inst.Address();
            shouldIncrementPC_ = false;
            break;
        case InstructionType::RegisterMaskedRandom:
            registerFile_[inst.RegisterX()] = std::rand() & inst.Immediate();
            break;
        case InstructionType::DrawSprite:
            auto x = registerFile_[inst.RegisterX()];
            auto y = registerFile_[inst.RegisterY()];
            drawSprite(x, y, inst.PixelHeight());
            break;
        }
    }

    void CPU::handleInstruction(MachineInstruction &inst)
    {
        using InstructionType = MachineInstruction::InstructionType;
        switch (inst.Type)
        {
        case InstructionType::MachineCall:
            callStack_.push(programCounter_);
            programCounter_ = inst.Address();
            shouldIncrementPC_ = false;
            break;
        case InstructionType::DisplayClear:
            std::memset(frameBuffer_.data(), 0, frameBuffer_.size());
            break;
        case InstructionType::Return:
            programCounter_ = callStack_.top();
            callStack_.pop();
            shouldIncrementPC_ = false;
            break;
        }
    }

    void CPU::handleInstruction(ALUInstruction &inst)
    {
        using InstructionType = ALUInstruction::InstructionType;

        switch (inst.Type)
        {
        case InstructionType::MOV:
            registerFile_[inst.RegisterX()] = registerFile_[inst.RegisterY()];
            break;
        case InstructionType::OR:
            registerFile_[inst.RegisterX()] |= registerFile_[inst.RegisterY()];
            break;
        case InstructionType::AND:
            registerFile_[inst.RegisterX()] &= registerFile_[inst.RegisterY()];
            break;
        case InstructionType::XOR:
            registerFile_[inst.RegisterX()] ^= registerFile_[inst.RegisterY()];
            break;
        case InstructionType::ADD:
            registerFile_[inst.RegisterX()] += registerFile_[inst.RegisterY()];
            break;
        case InstructionType::SUB:
            registerFile_[VF_] = registerFile_[inst.RegisterY()] > registerFile_[inst.RegisterX()]
                                     ? 0x00
                                     : 0x01;
            registerFile_[inst.RegisterX()] -= registerFile_[inst.RegisterY()];
            break;
        case InstructionType::SRL:
            registerFile_[VF_] = registerFile_[inst.RegisterX()] & 0x1;
            registerFile_[inst.RegisterX()] >>= 1;
            break;
        case InstructionType::NSUB:
            registerFile_[VF_] = registerFile_[inst.RegisterY()] > registerFile_[inst.RegisterX()]
                                     ? 0x01
                                     : 0x00;
            registerFile_[inst.RegisterX()] = registerFile_[inst.RegisterY()] - registerFile_[inst.RegisterX()];
            break;
        case InstructionType::SLL:
            registerFile_[VF_] = (registerFile_[inst.RegisterX()] >> 8) & 0x1;
            registerFile_[inst.RegisterX()] <<= 1;
            break;
        }
    }

    void CPU::handleInstruction(KeyInstruction &inst)
    {
        using InstructionType = KeyInstruction::InstructionType;
        switch (inst.Type)
        {
        case InstructionType::SkipIfPressed:
            if (keyState_[registerFile_[inst.RegisterX()]])
            {
                programCounter_ += 2;
            }
            break;
        case InstructionType::SkipIfNotPressed:
            if (!keyState_[registerFile_[inst.RegisterX()]])
            {
                programCounter_ += 2;
            }
            break;
        }
    }

    void CPU::handleInstruction(ControlInstruction &inst)
    {
        using InstructionType = ControlInstruction::InstructionType;
        switch (inst.Type)
        {
        case InstructionType::GetDelay:
        {
            std::lock_guard<std::mutex> lock(timerMutex_);
            registerFile_[inst.RegisterX()] = delayTimer_;
            break;
        }
        case InstructionType::AwaitAndGetKey:
            halted_ = true;
            registerAwaitingKey_ = inst.RegisterX();
            break;
        case InstructionType::SetDelayTimer:
        {
            std::lock_guard<std::mutex> lock(timerMutex_);
            delayTimer_ = registerFile_[inst.RegisterX()];
            break;
        }
        case InstructionType::SetSoundTimer:
        {
            std::lock_guard<std::mutex> lock(timerMutex_);
            soundTimer_ = registerFile_[inst.RegisterX()];
            break;
        }
        case InstructionType::IncrementAddress:
            indexRegister_ += registerFile_[inst.RegisterX()];
            break;
        case InstructionType::GetSpriteAddress:
            indexRegister_ = FONT_MEMORY_OFFSET + FONT_BYTES * registerFile_[inst.RegisterX()];
            break;
        case InstructionType::StoreBCD:
        {
            auto &reg = registerFile_[inst.RegisterX()];
            // doing a while loop would not be constant time
            memory_[indexRegister_ + 0] = (BCD_Table[reg] >> 8) & 0xF;
            memory_[indexRegister_ + 1] = (BCD_Table[reg] >> 4) & 0xF;
            memory_[indexRegister_ + 2] = (BCD_Table[reg] >> 0) & 0xF;
            break;
        }
        case InstructionType::RegisterDump:
            std::copy(registerFile_.begin(),
                      registerFile_.end(),
                      memory_.begin() + indexRegister_);
            break;
        case InstructionType::RegisterRestore:
            std::copy(memory_.begin() + indexRegister_,
                      memory_.begin() + indexRegister_ + registerFile_.size(),
                      registerFile_.begin());
            break;
        }
    }

    CPU::FrameBuffer CPU::GetFrameBuffer() const
    {
        FrameBuffer buf;

        for (size_t i = 0; i < SCR_HEIGHT; ++i)
        {
            for (size_t j = 0; j < SCR_WIDTH / 8; ++j)
            {
                for (int b = 0; b < 8; ++b)
                {
                    buf[i][8 * j + b] = (frameBuffer_[SCR_WIDTH / 8 * i + j] >> (7 - b)) & 0x1;
                }
            }
        }

        return buf;
    }

    void CPU::timerTick()
    {
        // auto now = std::chrono::high_resolution_clock::now();
        // auto delta = now - lastTimerTick_;
        // auto deltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();

        // // the clock runs at 60Hz so the number of ticks is
        // // ms * (60 ticks / 1000 ms)
        // auto ticks = static_cast<uint8_t>(deltaMs * 60.0 / 1000.0);

        // if (delayTimer_ > 0)
        //     delayTimer_ = std::max(0, delayTimer_ - ticks);
        // if (soundTimer_ > 0)
        //     delayTimer_ = std::max(0, soundTimer_ - ticks);

        // lastTimerTick_ = now;

        std::lock_guard<std::mutex> lock(timerMutex_);
        if (delayTimer_ > 0)
        {
            --delayTimer_;
        }
        if (soundTimer_ > 0)
        {
            --soundTimer_;
        }
    }

    void CPU::Cycle()
    {
        std::cerr << DumpState() << std::endl;

        systemClock_++;

        if (halted_)
        {
            if (!isKeyboardDirty())
            {
                return;
            }

            auto it = std::find(keyState_.begin(), keyState_.end(), true);
            if (it != keyState_.end())
            {
                registerFile_[registerAwaitingKey_] = it - keyState_.begin();
                halted_ = false;
            }
            else
            {
                return;
            }
        }

        auto instr_raw = currentInstruction();
        auto instr = DecodeInstruction(instr_raw);

        using InstructionType = Instruction::InstructionType;
        switch (instr->Type)
        {
        case InstructionType::MachineInstruction:
            handleInstruction(dynamic_cast<MachineInstruction &>(*instr));
            break;
        case InstructionType::ALUInstruction:
            handleInstruction(dynamic_cast<ALUInstruction &>(*instr));
            break;
        case InstructionType::KeyInstruction:
            handleInstruction(dynamic_cast<KeyInstruction &>(*instr));
            break;
        case InstructionType::ControlInstruction:
            handleInstruction(dynamic_cast<ControlInstruction &>(*instr));
            break;
        default:
            handleInstruction(*instr);
            break;
        }

        if (shouldIncrementPC_)
        {
            programCounter_ += 2;
        }
        else
        {
            shouldIncrementPC_ = true;
        }
    }

    void CPU::SetKeyState(uint8_t key, bool state)
    {
        keyState_[key] = state;
    }

    std::string CPU::dumpSpecial() const
    {
        std::stringstream ss;
        // PC, index
        ss << "PC: " << WordPrinter(programCounter_)
           << "\n"
           << "Index: " << WordPrinter(indexRegister_)
           << "\n";
        // timers
        ss << "Delay: " << BytePrinter(delayTimer_)
           << "\n"
           << "Sound: " << BytePrinter(soundTimer_)
           << "\n";

        return ss.str();
    }

    std::string CPU::dumpRegisters() const
    {
        std::stringstream ss;
        for (size_t i = 0; i < registerFile_.size(); ++i)
        {
            ss << "V" << i << ": " << BytePrinter(registerFile_[i]) << "\n";
        }

        return ss.str();
    }

    std::string CPU::dumpMemory() const
    {
        std::stringstream ss;
        for (size_t i = PROG_MEMORY_OFFSET; i < memory_.size(); ++i)
        {
            ss << PlainBytePrinter(memory_[i]) << " ";
            if (i % 16 == 15)
            {
                ss << "\n";
            }
        }

        return ss.str();
    }

    std::string CPU::dumpFrameBuffer() const
    {
        std::stringstream ss;
        uint8_t col = 0;
        for (const auto &byte : frameBuffer_)
        {
            for (int i = 0; i < 8; ++i)
            {
                ss << ((byte >> (7 - i) & 1) ? "1" : "0");
            }

            if (col % 8 == 7)
            {
                ss << "\n";
            }

            col = (col + 1) % 8;
        }

        return ss.str();
    }

    std::string CPU::dumpStack() const
    {
        std::stringstream ss;
        std::stack<uint16_t> stack(callStack_);
        for (size_t i = 0; i < stack.size(); ++i)
        {
            auto val = stack.top();
            stack.pop();
            ss << WordPrinter(val) << "\n";
        }

        return ss.str();
    }

    std::string CPU::dumpKeyboard() const
    {
        std::stringstream ss;
        for (size_t i = 0; i < keyState_.size(); ++i)
        {
            ss << (keyState_[i] ? "X" : "O");
        }

        return ss.str();
    }

    std::string CPU::DumpState() const
    {
        std::stringstream ss;
        ss << "=== CPU State Dump (cycle: " << systemClock_ << ") ===\n";
        ss << dumpSpecial()
           << "\n==REGISTERS==\n"
           << dumpRegisters()
           << "\n==MEMORY==\n"
           << dumpMemory()
           << "\n==FRAME BUFFER==\n"
           << dumpFrameBuffer()
           << "\n==STACK==\n"
           << dumpStack()
           << "\n==KEYBOARD==\n"
           << dumpKeyboard()
           << "\n\n";
        return ss.str();
    }
}
