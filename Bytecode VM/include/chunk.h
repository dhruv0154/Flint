#pragma once

#include "common.h"
#include "memory.h"

enum class OpCode : uint8_t {
    OP_RETURN,
};

class Chunk
{
private:
    int capacity;
    int count;
    uint8_t* code;
public:
    Chunk() : count(0), capacity(0), code(nullptr) {}
    
    void writeChunk(uint8_t byte);
    int getCount() { return count; }
    int getCapacity() { return capacity; }
    uint8_t* getCode() { return code; }

    ~Chunk() { Memory::freeArray(code, capacity); }
};