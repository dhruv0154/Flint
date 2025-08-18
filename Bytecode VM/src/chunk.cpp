#include "chunk.h"

void Chunk::writeChunk(uint8_t byte, int line)
{
    if(capacity < count + 1)
    {
        int oldCapacity = capacity;
        capacity = Memory::growCapacity(oldCapacity);
        code = Memory::growArray(code, oldCapacity, capacity);
        lines = Memory::growArray(lines, oldCapacity, capacity);
    }
    lines[count] = line;
    code[count] = byte;
    count++;
}

int Chunk::addConstant(Value val)
{
    constants.writeValueArray(val);
    return constants.getCount() - 1;    
}