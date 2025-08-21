#include "chunk.h"

void Chunk::writeChunk(uint8_t byte, int line)
{
    code.push_back(byte);

    if(lines.size() <= 0 || lines[lines.size() - 1].line != line)
    {
        lines.push_back({ line, static_cast<int>(code.size() - 1)});
    }
}

void Chunk::writeConstant(Value val, int line)
{
    int constantIndex = addConstant(val);

    if(constantIndex < 256)
    {
        writeChunk(static_cast<uint8_t>(OpCode::OP_CONSTANT), line);
        writeChunk(uint8_t(constantIndex), line);
    }
    else
    {
        writeChunk(static_cast<uint8_t>(OpCode::OP_CONSTANT_LONG), line);

        writeChunk((constantIndex << 0) & 0xFF, line);
        writeChunk((constantIndex << 8) & 0xFF, line);
        writeChunk((constantIndex << 16) & 0xFF, line);
    }
}

int Chunk::getLine(int instruction) 
{
    for (int i = lines.size() - 1; i >= 0; i--) 
    {
        if (instruction >= lines[i].offset) {
            return lines[i].line;
        }
    }
    return -1; // should never happen
}

int Chunk::addConstant(Value val)
{
    constants.push_back(val);
    return constants.size() - 1;    
}