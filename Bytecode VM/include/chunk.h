#pragma once

#include "common.h"
#include "memory.h"
#include "value.h"

enum class OpCode : uint8_t {
    OP_CONSTANT,
    OP_CONSTANT_LONG,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,
    OP_RETURN,
};

struct LineStart {
    int line;
    int offset;
};

class Chunk
{
private:
    DynamicArray<uint8_t> code;
    DynamicArray<LineStart> lines;
    ValueArray constants;
public:
    Chunk() = default;
    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;
    Chunk(Chunk&&) = delete;
    Chunk& operator=(Chunk&&) = delete;
    
    void writeChunk(uint8_t byte, int line);
    void writeConstant(Value val, int line);
    int addConstant(Value val);
    int getLine(int instruction);
    int getCount() { return code.size(); }
    ValueArray& getConstants() { return constants; }
    DynamicArray<LineStart>& getLines() { return lines; }
    DynamicArray<uint8_t>& getCode() { return code; }
};