#include <Utils/CHIP8ISA.h>

#include <unordered_map>
#include <functional>
#include <iomanip>

using namespace SKChip8;
using decoder = std::function<void(uint16_t, std::stringstream &)>;

namespace
{
    void _lower1hex(uint16_t opcode, std::stringstream &res)
    {
        res << "0x" << std::hex << std::setfill('0') << std::setw(1) << (opcode & 0xF);
    }

    void _lower2hex(uint16_t opcode, std::stringstream &res)
    {
        res << "0x" << std::hex << std::setfill('0') << std::setw(2) << (opcode & 0xFF);
    }

    void _lower3hex(uint16_t opcode, std::stringstream &res)
    {
        res << "0x" << std::hex << std::setfill('0') << std::setw(3) << (opcode & 0xFFF);
    }

    void _decodeXNN(uint16_t x, std::stringstream &res)
    {
        res << "\tV[";
        _lower1hex(x >> 8, res);
        res << "], ";
        _lower2hex(x, res);
    }

    void _decodeXY_(uint16_t x, std::stringstream &res)
    {
        res << "\tV[";
        _lower1hex(x >> 8, res);
        res << "], V[";
        _lower1hex(x >> 4, res);
        res << "]";
    }

    void _decodeXYN(uint16_t x, std::stringstream &res)
    {
        res << "\tV[";
        _lower1hex(x >> 8, res);
        res << "], V[";
        _lower1hex(x >> 4, res);
        res << "], ";
        _lower1hex(x, res);
    }

    void decodeMachineCall(uint16_t x, std::stringstream &res)
    {
        res << "MCALL\t";
        _lower3hex(x, res);
    }

    void decodeDisplayClear(uint16_t x, std::stringstream &res)
    {
        res << "DISP_CLEAR";
    }

    void decodeReturnFromSubroutine(uint16_t x, std::stringstream &res)
    {
        res << "RET";
    }

    void decodeGoto(uint16_t x, std::stringstream &res)
    {
        res << "goto\t";
        _lower3hex(x, res);
    }

    void decodeCall(uint16_t x, std::stringstream &res)
    {
        res << "call\t";
        _lower3hex(x, res);
    }

    void decodeSkipIfEqual(uint16_t x, std::stringstream &res)
    {
        res << "SKIP_EQ";
        _decodeXNN(x, res);
    }

    void decodeSkipIfNotEqual(uint16_t x, std::stringstream &res)
    {
        res << "SKIP_NEQ";
        _decodeXNN(x, res);
    }

    void decodeSkipIfRegistersEqual(uint16_t x, std::stringstream &res)
    {
        res << "SKIP_REQ";
        _decodeXY_(x, res);
    }

    void decodeAssignImmediate(uint16_t x, std::stringstream &res)
    {
        res << "MOV";
        _decodeXNN(x, res);
    }

    void decodeAddImmediate(uint16_t x, std::stringstream &res)
    {
        res << "ADD";
        _decodeXNN(x, res);
    }

    static std::unordered_map<uint16_t, std::string> alu_opname;
    static std::unordered_map<uint8_t, std::string> suffix_decoder;
    static std::unordered_map<uint16_t, decoder> immediate_decoders;
    void init_decoders()
    {
        alu_opname[0] = "MOV";
        alu_opname[1] = "OR";
        alu_opname[2] = "AND";
        alu_opname[3] = "XOR";
        alu_opname[4] = "ADD";
        alu_opname[5] = "SUB";
        alu_opname[6] = "SHR";
        alu_opname[7] = "SUBN";
        alu_opname[0xE] = "SHL";

        suffix_decoder[0x07] = "GET_DELAY";
        suffix_decoder[0x0A] = "GET_KEY";
        suffix_decoder[0x15] = "SET_DELAY";
        suffix_decoder[0x18] = "SET_SOUND";
        suffix_decoder[0x1E] = "ADD_I";
        suffix_decoder[0x29] = "SET_FONT";
        suffix_decoder[0x33] = "BCD";
        suffix_decoder[0x55] = "MEM_DUMP";
        suffix_decoder[0x65] = "MEM_LOAD";

        immediate_decoders[0x00E0] = decodeDisplayClear;
        immediate_decoders[0x00EE] = decodeReturnFromSubroutine;
    }

    void decodeRegisterALUOp(uint16_t x, std::stringstream &res)
    {
        res << alu_opname[x & 0xF];
        _decodeXY_(x, res);
    }

    void decodeSkipIfRegistersNotEqual(uint16_t x, std::stringstream &res)
    {
        res << "SKIP_RNEQ\t";
        _decodeXY_(x, res);
    }

    void decodeAssignAddress(uint16_t x, std::stringstream &res)
    {
        res << "MOVI\t";
        _lower3hex(x, res);
    }

    void decodeJumpAdd(uint16_t x, std::stringstream &res)
    {
        res << "JMPI\t";
        _lower3hex(x, res);
    }

    void decodeRandom(uint16_t x, std::stringstream &res)
    {
        res << "RAND";
        _decodeXNN(x, res);
    }

    void decodeDraw(uint16_t x, std::stringstream &res)
    {
        res << "DRAW";
        _decodeXYN(x, res);
    }

    void decodeKeyOp(uint16_t x, std::stringstream &res)
    {
        // TODO(sk00) need to check if A1?
        res << (x & 0xFF == 0x9E ? "KEY_EQ\t" : "KEY_NEQ\t");
        _lower1hex(x >> 8, res);
    }

    void decodeFOps(uint16_t x, std::stringstream &res)
    {
        res << suffix_decoder[static_cast<uint8_t>(x & 0xFF)] << "\t";
        _lower1hex(x >> 8, res);
    }
}

static std::array<decoder, 16> prefix_decoders{
    decodeMachineCall,
    decodeGoto,
    decodeCall,
    decodeSkipIfEqual,
    decodeSkipIfNotEqual,
    decodeSkipIfRegistersEqual,
    decodeAssignImmediate,
    decodeAddImmediate,
    decodeRegisterALUOp,
    decodeSkipIfRegistersNotEqual,
    decodeAssignAddress,
    decodeJumpAdd,
    decodeRandom,
    decodeDraw,
    decodeKeyOp,
    decodeFOps};

std::string SKChip8::DecodeInstruction(uint16_t opcode)
{
    init_decoders();

    std::stringstream res;
    if (immediate_decoders.find(opcode) != immediate_decoders.end())
    {

        immediate_decoders[opcode](opcode, res);
    }
    else
    {
        prefix_decoders[(opcode & 0xF000) >> 12](opcode, res);
    }

    return res.str();
}