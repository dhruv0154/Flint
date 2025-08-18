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

    uint8_t instruction = chunk->getCode()[offset];
    switch (instruction)
    {
    case static_cast<uint8_t>(OpCode::OP_RETURN):
        return simpleInstruction("OP_RETURN", offset);
    default:
        std::cout << "Unknown opcode: " << instruction;
        return offset + 1;
    }
}