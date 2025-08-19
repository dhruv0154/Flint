#pragma once

#include "common.h"
#include "memory.h"
#include "value.h"

enum class OpCode : uint8_t {
    OP_CONSTANT,
    OP_CONSTANT_LONG,
    OP_RETURN,
};

struct LineStart {
    int line;
    int offset;
};

class Chunk
{
private:
    int capacity;
    int count;
    uint8_t* code;

    int lineCount;
    int lineCapacity;
    LineStart* lines;
    ValueArray constants;
public:
    Chunk() : count(0), capacity(0), 
        code(nullptr), lines(nullptr), lineCount(0), lineCapacity(0), constants() {}
    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;
    Chunk(Chunk&&) = delete;
    Chunk& operator=(Chunk&&) = delete;
    
    void writeChunk(uint8_t byte, int line);
    void writeConstant(Value val, int line);
    int addConstant(Value val);
    int getLine(int instruction);
    int getCount() { return count; }
    int getCapacity() { return capacity; }
    ValueArray& getConstants() { return constants; }
    LineStart* getLines() { return lines; }
    uint8_t* getCode() { return code; }

    ~Chunk() { 
        Memory::freeArray(code); 
        Memory::freeArray(lines);
    }
};