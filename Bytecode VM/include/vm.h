#pragma once

#include "chunk.h"

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
    Value stack[STACK_MAX];
    Value* stackTop;
    InterpretResult run();
public:
    VM() = default;
    InterpretResult interpret(Chunk* chunk);
    ~VM() = default;
};