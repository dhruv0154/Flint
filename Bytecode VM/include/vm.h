#pragma once

#include "chunk.h"
#include "memory.h"
#include "compiler.h"

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
    Compiler compiler;
    VM() = default;
    InterpretResult interpret(const std::string &source);
    ~VM() = default;
};