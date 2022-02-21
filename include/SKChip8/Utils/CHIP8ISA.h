#ifndef _CHIP8ISA_H
#define _CHIP8ISA_H

#include <string>
#include <cstdint>
#include <memory>

namespace SKChip8
{
    class Instruction;
    class MachineInstruction;
    class ALUInstruction;
    class KeyInstruction;
    class ControlInstruction;

    std::unique_ptr<Instruction> DecodeInstruction(uint16_t opcode);

    class Instruction
    {
    public:
        // comes from the top 4 bits
        enum class InstructionType
        {
            MachineInstruction = 0x0,
            ALUInstruction = 0x8,
            KeyInstruction = 0xE,
            ControlInstruction = 0xF,

            // Jump to address NNN
            Goto = 0x1,
            // Call subroutine NNN
            Call = 0x2,
            // Skip next instruction if VX==NN
            SkipIfEqual = 0x3,
            // Skip next instruction if VX!=NN
            SkipIfNotEqual = 0x4,
            // Skip if Vx == Vy
            SkipIfRegistersEqual = 0x5,
            // VX <- NN
            MoveRegisterXImmediate = 0x6,
            // Vx += NN
            AddRegisterImmediate = 0x7,
            // Skip next instruction if Vx != Vy
            SkipIfNotEqualRegisters = 0x9,
            // I <- NNN
            SetAddressImmediate = 0xA,
            // PC <- V0 + NNN
            JumpLong = 0xB,
            // Vx <- rand() & NN
            RegisterMaskedRandom = 0xC,
            // Draw a sprite at (Vx,Vy) with a height of N pixels
            // and a width of 8. sprite is located at address I
            // and Vf is set to 1 if any pixels go from set to unset
            DrawSprite = 0xD,

        };

        Instruction(uint16_t opcode) : opcode_(opcode),
                                       Type(InstructionType((opcode >> 12) & 0xF)) {}

        const InstructionType Type;

        uint16_t Address() const;
        uint8_t RegisterX() const;
        uint8_t RegisterY() const;
        uint8_t Immediate() const;
        uint8_t PixelHeight() const;

        virtual std::ostream &dump(std::ostream &os) const;

        friend std::ostream &operator<<(std::ostream &os, const Instruction &instr);

    private:
        uint16_t opcode_;
    };

    struct MachineInstruction : public Instruction
    {
        // Comes from the lower 12 bits (default is Call)
        enum class InstructionType
        {
            // Call machine code routine at NNN
            MachineCall,
            // Clear the screen
            DisplayClear = 0x0E0,
            // Return from subroutine
            Return = 0x0EE
        };

        // TODO(sk00) decode machine call correctly
        MachineInstruction(uint16_t opcode) : Instruction(opcode),
                                              Type(InstructionType(opcode & 0xFFF)) {}

        const InstructionType Type;

        std::ostream &dump(std::ostream &os) const override;
    };

    struct ALUInstruction : public Instruction
    {
        // Comes from the lower 4 bits
        enum class InstructionType
        {
            // Vx = Vy
            MOV = 0x0,
            // Vx |= Vy
            OR = 0x1,
            // Vx &= Vy
            AND = 0x2,
            // Vx ^= Vy
            XOR = 0x3,
            // Vx += Vy
            ADD = 0x4,
            // Vx -= Vy
            SUB = 0x5,
            // Vf <- Vx[0], Vx >>= 1
            SRL = 0x6,
            // Vx = Vy - Vx, Vf <- 0 if borrow else 1
            NSUB = 0x7,
            // Vf <- Vx[7], Vx <<= 1
            SLL = 0xE
        };

        ALUInstruction(uint16_t opcode) : Instruction(opcode),
                                          Type(InstructionType(opcode & 0xF)) {}

        const InstructionType Type;

        std::ostream &dump(std::ostream &os) const override;
    };

    struct KeyInstruction : public Instruction
    {
        // Comes from the lower 8 bits
        enum class InstructionType
        {
            // Skip the next instruction if key [Vx] is pressed
            SkipIfPressed = 0x9E,
            // Skip the next instruction if key [Vx] is NOT pressed
            SkipIfNotPressed = 0xA1
        };

        KeyInstruction(uint16_t opcode) : Instruction(opcode),
                                          Type(InstructionType(opcode & 0xFF)) {}

        const InstructionType Type;

        std::ostream &dump(std::ostream &os) const override;
    };

    struct ControlInstruction : public Instruction
    {
        // Comes from the lower 8 bits
        enum class InstructionType
        {
            // Set Vx to the delay timer value
            GetDelay = 0x07,
            // Await key press, store value in Vx (blocking)
            AwaitAndGetKey = 0x0A,
            // Set the delay timer to Vx
            SetDelayTimer = 0x15,
            // Set the sound timer to Vx
            SetSoundTimer = 0x18,
            // I += Vx
            IncrementAddress = 0x1E,
            // Set I to the location of the 4x5 font sprite for character [Vx]
            GetSpriteAddress = 0x29,
            // Store BCD of Vx at I (big endian: MSB at I LSB at I+2)
            StoreBCD = 0x33,
            // Dumps registers [V0, Vx] in memory at I
            RegisterDump = 0x55,
            // Fills registers [V0, Vx] starting at address I
            RegisterRestore = 0x65
        };

        ControlInstruction(uint16_t opcode) : Instruction(opcode),
                                              Type(InstructionType(opcode & 0xFF)) {}

        const InstructionType Type;

        std::ostream &dump(std::ostream &os) const override;
    };
}

#endif