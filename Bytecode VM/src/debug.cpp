#include "debug.h"
#include <iomanip>

int Disassembler::simpleInstruction(const char* name, int offset) {
    std::cout << name << std::endl;
    return offset + 1;
}

void Disassembler::disassembleChunk(Chunk* chunk, const char* name)
{  
    std::cout << name << std::endl;
    for (int offset = 0; offset < chunk->getCount();) {
        offset = disassembleInstruction(chunk, offset);
    }
}

int Disassembler::disassembleInstruction(Chunk* chunk, int offset)
{
    std::cout << std::setfill('0') << std::setw(4) << offset << " ";
    if(offset > 0 && chunk -> getLine(offset) == 
        chunk -> getLine(offset - 1))
    {
        std::cout << "   | ";
    }
    else
    {
        std::cout << std::setw(4) << std::setfill(' ') <<  chunk -> getLine(offset) << " ";
    }
    uint8_t instruction = chunk->getCode()[offset];
    OpCode instructionCode = static_cast<OpCode>(instruction);
    switch (instructionCode)
    {
    case OpCode::OP_RETURN:
        return simpleInstruction("OP_RETURN", offset);
    case OpCode::OP_CONSTANT:
        return constantInstruction("OP_CONSTANT", chunk, offset);
    case OpCode::OP_CONSTANT_LONG:
        return longConstantInstruction("OP_CONSTANT_LONG", chunk, offset);
    case OpCode::OP_NEGATE:
        return simpleInstruction("OP_NEGATE", offset);
    case OpCode::OP_ADD:
        return simpleInstruction("OP_ADD", offset);
    case OpCode::OP_SUBTRACT:
        return simpleInstruction("OP_SUBTRACT", offset);
    case OpCode::OP_MULTIPLY:
        return simpleInstruction("OP_MULTIPLY", offset);
    case OpCode::OP_DIVIDE:
        return simpleInstruction("OP_DIVIDE", offset);
    default:
        std::cout << "Unknown opcode: " << instruction;
        return offset + 1;
    }
}

int Disassembler::constantInstruction(const char* name, Chunk* chunk, int offset)
{
    uint8_t constantIndex = chunk -> getCode()[offset + 1];
    std::cout << std::left << std::setw(16) << name 
    << std::right << std::setw(4) << (int)constantIndex << " '";

    ValueArray& constants = chunk -> getConstants();
    std::cout << (constants[constantIndex]);
    std::cout << "'\n";
    return offset + 2;
}

int Disassembler::longConstantInstruction(const char* name, Chunk* chunk, int offset)
{
    uint32_t constantIndex = 
        chunk -> getCode()[offset + 1] |
        chunk -> getCode()[offset + 2] << 8 |
        chunk -> getCode()[offset + 3] << 16;

    std::cout << std::left << std::setw(16) << name 
    << std::right << std::setw(4) << (int)constantIndex << " '";

    ValueArray& constants = chunk -> getConstants();
    std::cout << (constants[constantIndex]);
    std::cout << "'\n";

    return offset + 4;
}