#include "debug.h"
#include <iomanip>

int Disassembler::simpleInstruction(const char* name, int offset) {
    std::cout << name;
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
    if(offset > 0 && chunk -> getLines()[offset] == 
        chunk -> getLines()[offset - 1])
    {
        std::cout << "   | ";
    }
    else
    {
        std::cout << std::setw(4) << std::setfill(' ') <<  chunk -> getLines()[offset] << " ";
    }
    uint8_t instruction = chunk->getCode()[offset];
    switch (instruction)
    {
    case static_cast<uint8_t>(OpCode::OP_RETURN):
        return simpleInstruction("OP_RETURN", offset);
    case static_cast<uint8_t>(OpCode::OP_CONSTANT):
        return constantInstruction("OP_CONSTANT", chunk, offset);
    default:
        std::cout << "Unknown opcode: " << instruction;
        return offset + 1;
    }
}

int Disassembler::constantInstruction(const char* name, Chunk* chunk, int offset)
{
    uint8_t constant = chunk -> getCode()[offset + 1];
    std::cout << std::left << std::setw(16) << name 
    << std::right << std::setw(4) << (int)constant << " '";

    ValueArray& constants = chunk -> getConstants();
    constants.printValue(constants.getValues()[constant]);
    std::cout << "'\n";
    return offset + 2;
}