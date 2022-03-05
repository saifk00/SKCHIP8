#include "CPU.h"

#include <Utils/CHIP8Utils.h>

#include <cstdlib>
#include <ctime>
#include <cstring>
#include <algorithm>

static constexpr uint16_t FONT_BYTES = 5;
static constexpr uint16_t FONT_DATA_SIZE = FONT_BYTES * 16;

static constexpr uint8_t font_data[FONT_DATA_SIZE] = {
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

// special registers implicitly used in some instructions
static constexpr uint8_t VF_ = 0xF;
static constexpr uint8_t V0_ = 0x0;

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
    }

    void CPU::LoadROM(std::vector<uint8_t> buffer)
    {
        std::copy(buffer.begin(), buffer.end(), memory_.begin() + PROG_MEMORY_OFFSET);
        programCounter_ = PROG_MEMORY_OFFSET;
        shouldIncrementPC_ = true;
    }

    uint16_t CPU::currentInstruction() const
    {
        return memory_[programCounter_] << 8 | memory_[programCounter_ + 1];
    }

    void CPU::drawSprite(uint8_t x, uint8_t y, uint8_t n)
    {
        const uint8_t offset = x % 8;
        bool collision = false;
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
            if (!collision)
            {
                collision = (frameBuffer_[frameIndex] & spriteThisPart);
            }
            frameBuffer_[frameIndex] ^= spriteThisPart;

            // the sprite may overflow to the next byte
            if (offset > 0 && frameIndex < FRAME_BUFFER_SIZE - 1)
            {
                if (!collision)
                {
                    collision = (frameBuffer_[frameIndex + 1] & spriteNextPart);
                }
                frameBuffer_[frameIndex + 1] ^= spriteNextPart;
            }
        }

        registerFile_[VF_] = collision ? 1 : 0;
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
            callStack_.push(programCounter_ + 2);
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
            // coordinates wrap around the screen
            auto x = registerFile_[inst.RegisterX()] % SCR_WIDTH;
            auto y = registerFile_[inst.RegisterY()] % SCR_HEIGHT;
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
            callStack_.push(programCounter_ + 2);
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
                programCounter_ += 4;
                shouldIncrementPC_ = false;
            }
            break;
        case InstructionType::SkipIfNotPressed:
            if (!keyState_[registerFile_[inst.RegisterX()]])
            {
                programCounter_ += 4;
                shouldIncrementPC_ = false;
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
            registerFile_[inst.RegisterX()] = delayTimer_;
            break;
        case InstructionType::AwaitAndGetKey:
            halted_ = true;
            registerAwaitingKey_ = inst.RegisterX();
            break;
        case InstructionType::SetDelayTimer:
            delayTimer_ = registerFile_[inst.RegisterX()];
            break;
        case InstructionType::SetSoundTimer:
            soundTimer_ = registerFile_[inst.RegisterX()];
            break;
        case InstructionType::IncrementAddress:
            indexRegister_ += registerFile_[inst.RegisterX()];
            break;
        case InstructionType::GetSpriteAddress:
            indexRegister_ = FONT_MEMORY_OFFSET + FONT_BYTES * registerFile_[inst.RegisterX()];
            break;
        case InstructionType::StoreBCD:
        {
            // hundreds, tens, ones starting at index
            auto &reg = registerFile_[inst.RegisterX()];
            memory_[indexRegister_ + 0] = reg / 100;
            memory_[indexRegister_ + 1] = (reg / 10) % 10;
            memory_[indexRegister_ + 2] = reg % 10;
            break;
        }
        case InstructionType::RegisterDump:
            std::copy(registerFile_.begin(),
                      registerFile_.begin() + inst.RegisterX() + 1,
                      memory_.begin() + indexRegister_);
            break;
        case InstructionType::RegisterRestore:
            std::copy(memory_.begin() + indexRegister_,
                      memory_.begin() + indexRegister_ + inst.RegisterX() + 1,
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

    void CPU::TimerTick()
    {
        if (delayTimer_ > 0)
        {
            delayTimer_--;
        }
        if (soundTimer_ > 0)
        {
            soundTimer_--;
        }
    }

    void CPU::Cycle()
    {
        // std::cerr << DumpState() << std::endl;
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
