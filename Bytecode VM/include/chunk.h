#pragma once

#include "common.h"
#include "memory.h"
#include "value.h"

enum class OpCode : uint8_t {
    OP_CONSTANT,
    OP_RETURN,
};

class Chunk
{
private:
    int capacity;
    int count;
    uint8_t* code;
    int* lines;
    ValueArray constants;
public:
    Chunk() : count(0), capacity(0), code(nullptr), lines(nullptr), constants() {}
    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;
    Chunk(Chunk&&) = delete;
    Chunk& operator=(Chunk&&) = delete;
    
    void writeChunk(uint8_t byte, int line);
    int addConstant(Value val);
    int getCount() { return count; }
    int getCapacity() { return capacity; }
    ValueArray& getConstants() { return constants; }
    int* getLines() { return lines; }
    uint8_t* getCode() { return code; }

    ~Chunk() { 
        Memory::freeArray(code); 
        Memory::freeArray(lines);
    }
};