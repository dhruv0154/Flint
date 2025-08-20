#pragma once

#include "chunk.h"

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
    InterpretResult run();
public:
    VM();
    InterpretResult interpret(Chunk* chunk);
    ~VM();
};