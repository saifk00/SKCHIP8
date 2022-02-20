#include "CPU.h"

#include <cstdlib>
#include <ctime>

namespace SKChip8
{
    // TODO(sk00) dependency inject a frame and audio buffer to connect this to display,
    // audio, input, and timers
    CPU::CPU()
    {
        // initialize timers, peripherals, etc.
        std::srand(std::time(0));
    }

    void CPU::LoadROM(std::vector<uint8_t> buffer)
    {
        std::copy(buffer.begin(), buffer.end(), memory_.begin() + PROG_MEMORY_OFFSET);
        programCounter_ = PROG_MEMORY_OFFSET;
    }

    uint8_t &CPU::currentInstruction()
    {
        return memory_.at(programCounter_);
    }

    void CPU::handleInstruction(Instruction &inst)
    {
        using InstructionType = Instruction::InstructionType;
        switch (inst.Type)
        {
        case InstructionType::Goto:
            programCounter_ = inst.Address();
            break;
        case InstructionType::Call:
            // TODO(sk00) do anything ddifferent than goto?
            programCounter_ = inst.Address();
            break;
        case InstructionType::SkipIfEqual:
            if (registerFile_[inst.RegisterX()] == inst.Immediate())
            {
                programCounter_ += 2;
            }
            break;
        case InstructionType::SkipIfNotEqual:
            if (registerFile_[inst.RegisterX()] != inst.Immediate())
            {
                programCounter_ += 2;
            }
            break;
        case InstructionType::SkipIfRegistersEqual:
            if (registerFile_[inst.RegisterX()] == registerFile_[inst.RegisterY()])
            {
                programCounter_ += 2;
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
                programCounter_ += 2;
            }
            break;
        case InstructionType::SetAddressImmediate:
            indexRegister_ = inst.Address();
            break;
        case InstructionType::JumpLong:
            programCounter_ = registerFile_[0] + inst.Address();
            break;
        case InstructionType::RegisterMaskedRandom:
            registerFile_[inst.RegisterX()] = std::rand() & inst.Immediate();
            break;
        case InstructionType::DrawSprite:
            // TODO(sk00) draw a sprite
            break;
        }
    }

    void CPU::DoCycle()
    {
        auto &instr_raw = currentInstruction();
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
    }
}