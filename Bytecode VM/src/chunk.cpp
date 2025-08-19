#include "chunk.h"

void Chunk::writeChunk(uint8_t byte, int line)
{
    if(capacity < count + 1)
    {
        int oldCapacity = capacity;
        capacity = Memory::growCapacity(oldCapacity);
        code = Memory::growArray(code, oldCapacity, capacity);
    }
    code[count] = byte;

    if(lineCount <= 0 || lines[lineCount - 1].line != line)
    {
        if(lineCapacity < lineCount + 1)
        {
            int oldCapacity = lineCapacity;
            lineCapacity = Memory::growCapacity(oldCapacity);
            lines = Memory::growArray(lines, oldCapacity, lineCapacity);
        }
        lines[lineCount].line = line;
        lines[lineCount].offset = count;
        lineCount++;
    }
    count++;
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
    for (int i = lineCount - 1; i >= 0; i--) 
    {
        if (instruction >= lines[i].offset) {
            return lines[i].line;
        }
    }
    return -1; // should never happen
}

int Chunk::addConstant(Value val)
{
    constants.writeValueArray(val);
    return constants.getCount() - 1;    
}