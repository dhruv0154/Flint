#pragma once

#include "chunk.h"
#include "memory.h"

#define STACK_MAX 256

enum class InterpretResult
{ 
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
};

class VM
{
private:
    Chunk* chunk;
    uint8_t* ip;
    DynamicArray<Value> stack;
    InterpretResult run();
    template<typename Op>
    inline void binaryOp(Op op);
    
public:
    VM() = default;
    InterpretResult interpret(const std::string source);
    ~VM() = default;
};