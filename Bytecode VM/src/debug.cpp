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
    switch (instruction)
    {
    case static_cast<uint8_t>(OpCode::OP_RETURN):
        return simpleInstruction("OP_RETURN", offset);
    case static_cast<uint8_t>(OpCode::OP_CONSTANT):
        return constantInstruction("OP_CONSTANT", chunk, offset);
     case static_cast<uint8_t>(OpCode::OP_CONSTANT_LONG):
        return longConstantInstruction("OP_CONSTANT_LONG", chunk, offset);
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
    constants.printValue(constants.getValues()[constantIndex]);
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
    constants.printValue(constants.getValues()[constantIndex]);
    std::cout << "'\n";

    return offset + 4;
}