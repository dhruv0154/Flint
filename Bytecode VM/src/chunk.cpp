#include "chunk.h"

void Chunk::writeChunk(uint8_t byte)
{
    if(capacity < count + 1)
    {
        int oldCapacity = capacity;
        capacity = Memory::growCapacity(oldCapacity);
        code = Memory::growArray(code, oldCapacity, capacity);
    }
    code[count++] = byte;
}