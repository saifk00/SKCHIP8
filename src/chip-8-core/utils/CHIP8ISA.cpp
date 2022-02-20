#include <Utils/CHIP8ISA.h>

#include <iostream>
#include <string>
#include <iomanip>

namespace
{
    struct HexPrinter
    {
        HexPrinter(uint16_t val, uint8_t bits) : Bits(bits), Value(val){};

        friend std::ostream &operator<<(std::ostream &os, const HexPrinter &printer)
        {
            return os << std::hex << std::setfill('0') << std::setw(printer.Bits / 4) << printer.Value;
        }

        uint16_t Value;
        uint8_t Bits;
    };

    struct RegisterPrinter
    {
        HexPrinter _printer;
        RegisterPrinter(uint8_t val) : _printer(val, 4) {}

        friend std::ostream &operator<<(std::ostream &os, const RegisterPrinter &reg)
        {
            return os << "V" << reg._printer;
        }
    };

    struct AddressPrinter
    {
        HexPrinter _printer;
        AddressPrinter(uint16_t val) : _printer(val, 12) {}

        friend std::ostream &operator<<(std::ostream &os, const AddressPrinter &addr)
        {
            return os << "0x" << addr._printer;
        }
    };

    struct ImmediatePrinter
    {
        HexPrinter _printer;
        ImmediatePrinter(uint8_t val) : _printer(val, 8) {}

        friend std::ostream &operator<<(std::ostream &os, const ImmediatePrinter &imm)
        {
            return os << "$" << imm._printer;
        }
    };

    struct PixelHeightPrinter
    {
        HexPrinter _printer;
        PixelHeightPrinter(uint8_t val) : _printer(val, 4) {}

        friend std::ostream &operator<<(std::ostream &os, const PixelHeightPrinter &imm)
        {
            return os << "%" << imm._printer;
        }
    };
}

namespace SKChip8
{

    std::unique_ptr<Instruction> DecodeInstruction(uint16_t opcode)
    {
        auto basetype = Instruction::InstructionType((opcode >> 12) & 0xF);

        switch (basetype)
        {
        case Instruction::InstructionType::MachineInstruction:
            return std::make_unique<MachineInstruction>(opcode);
        case Instruction::InstructionType::ALUInstruction:
            return std::make_unique<ALUInstruction>(opcode);
        case Instruction::InstructionType::KeyInstruction:
            return std::make_unique<KeyInstruction>(opcode);
        case Instruction::InstructionType::ControlInstruction:
            return std::make_unique<ControlInstruction>(opcode);
        default:
            return std::make_unique<Instruction>(opcode);
        }
    }

    uint16_t Instruction::Address() const { return opcode_ & 0xFFF; }
    uint8_t Instruction::RegisterX() const { return (opcode_ >> 8) & 0xF; }
    uint8_t Instruction::RegisterY() const { return (opcode_ >> 4) & 0xF; }
    uint8_t Instruction::Immediate() const { return opcode_ & 0xFF; }
    uint8_t Instruction::PixelHeight() const { return opcode_ & 0xF; }
    std::ostream &Instruction::dump(std::ostream &os) const
    {
        switch (Type)
        {
        case InstructionType::Goto:
            return os << "GOTO\t\t" << AddressPrinter(Address());
        case InstructionType::Call:
            return os << "CALL\t\t" << AddressPrinter(Address());
        case InstructionType::SkipIfEqual:
            return os << "SKIPEQ\t" << RegisterPrinter(RegisterX()) << "\t" << ImmediatePrinter(Immediate());
        case InstructionType::SkipIfNotEqual:
            return os << "SKIPNEQ\t" << RegisterPrinter(RegisterX()) << "\t" << ImmediatePrinter(Immediate());
        case InstructionType::SkipIfRegistersEqual:
            return os << "SKIPEQ\t" << RegisterPrinter(RegisterX()) << "\t" << RegisterPrinter(RegisterY());
        case InstructionType::MoveRegisterXImmediate:
            return os << "MOV\t" << RegisterPrinter(RegisterX()) << "\t" << ImmediatePrinter(Immediate());
        case InstructionType::AddRegisterImmediate:
            return os << "ADD\t" << RegisterPrinter(RegisterX()) << "\t" << ImmediatePrinter(Immediate());
        case InstructionType::SkipIfNotEqualRegisters:
            return os << "SKIPNEQ\t" << RegisterPrinter(RegisterX()) << "\t" << RegisterPrinter(RegisterY());
        case InstructionType::SetAddressImmediate:
            return os << "SETADDR\t\t" << ImmediatePrinter(Immediate());
        case InstructionType::JumpLong:
            return os << "LJUMP\t\t" << AddressPrinter(Address());
        case InstructionType::RegisterMaskedRandom:
            return os << "RAND\t" << ImmediatePrinter(Immediate());
        case InstructionType::DrawSprite:
            return os << "DRAW\t" << RegisterPrinter(RegisterX())
                      << "\t" << RegisterPrinter(RegisterY())
                      << "\t" << PixelHeightPrinter(PixelHeight());
        default:
            throw std::invalid_argument("INVALID INSTRUCTION");
            // return os << "Instruction " << HexPrinter(opcode_, 16) << '\n'
            //           << '\t' << "Address: " << HexPrinter(Address(), 12) << '\n'
            //           << '\t' << "VX: " << RegisterPrinter(RegisterX()) << '\n'
            //           << '\t' << "VY: " << HexPrinter(RegisterY(), 4) << '\n'
            //           << '\t' << "Immediate: " << HexPrinter(Immediate(), 8) << '\n'
            //           << '\t' << "PixelHeight: " << HexPrinter(PixelHeight(), 4);
        }
    }

    std::ostream &MachineInstruction::dump(std::ostream &os) const
    {
        switch (Type)
        {
        case InstructionType::MachineCall:
            return os << "MCALL\t\t" << AddressPrinter(Address());
        case InstructionType::DisplayClear:
            return os << "CDISP";
        case InstructionType::Return:
            return os << "RET";
        default:
            throw std::invalid_argument("INVALID MACHINE INSTRUCTION");
        }
    }

    std::ostream &ALUInstruction::dump(std::ostream &os) const
    {
        std::string instr;
        bool printy = true;

        switch (Type)
        {
        case InstructionType::MOV:
            instr = "MOV";
            break;
        case InstructionType::OR:
            instr = "OR";
            break;
        case InstructionType::AND:
            instr = "AND";
            break;
        case InstructionType::XOR:
            instr = "XOR";
            break;
        case InstructionType::ADD:
            instr = "ADD";
            break;
        case InstructionType::SUB:
            instr = "SUB";
            break;
        case InstructionType::SRL:
            instr = "SRL";
            printy = false;
            break;
        case InstructionType::NSUB:
            instr = "NSUB";
            break;
        case InstructionType::SLL:
            instr = "SLL";
            printy = false;
            break;
        default:
            throw std::invalid_argument("INVALID ALU INSTRUCTION");
        }

        os << instr << (printy ? "\t" : "\t\t") << RegisterPrinter(RegisterX());
        if (printy)
        {
            return os << "\t" << RegisterPrinter(RegisterY());
        }
        else
        {
            return os;
        }
    }

    std::ostream &KeyInstruction::dump(std::ostream &os) const
    {
        switch (Type)
        {
        case InstructionType::SkipIfPressed:
            return os << "SKPRESS\t\t" << RegisterPrinter(RegisterX());
        case InstructionType::SkipIfNotPressed:
            return os << "SKNPRESS\t\t" << RegisterPrinter(RegisterY());
        default:
            throw std::invalid_argument("INVALID KEY INSTRUCTION");
        }
    }

    std::ostream &ControlInstruction::dump(std::ostream &os) const
    {
        switch (Type)
        {
        case InstructionType::GetDelay:
            return os << "GETDELAY\t\t" << RegisterPrinter(RegisterX());
        case InstructionType::AwaitAndGetKey:
            return os << "GETKEY\t\t" << RegisterPrinter(RegisterX());
        case InstructionType::SetDelayTimer:
            return os << "SETDELAY\t\t" << RegisterPrinter(RegisterX());
        case InstructionType::SetSoundTimer:
            return os << "SETSOUND\t\t" << RegisterPrinter(RegisterX());
        case InstructionType::IncrementAddress:
            return os << "INCRADDR\t\t" << RegisterPrinter(RegisterX());
        case InstructionType::GetSpriteAddress:
            return os << "GETFONT\t\t" << RegisterPrinter(RegisterX());
        case InstructionType::StoreBCD:
            return os << "BCD\t\t" << RegisterPrinter(RegisterX());
        case InstructionType::RegisterDump:
            return os << "SAVE\t\t" << RegisterPrinter(RegisterX());
        case InstructionType::RegisterRestore:
            return os << "LOAD\t\t" << RegisterPrinter(RegisterX());
        default:
            throw std::invalid_argument("INVALID CONTROL INSTRUCTION");
        }
    }

    std::ostream &operator<<(std::ostream &os, const Instruction &instr)
    {
        return instr.dump(os);
    }
}