#pragma once
#include <iostream>
#include "chunk.h"

class Disassembler
{
public:
    void disassembleChunk(Chunk* chunk, const char* name);
    int disassembleInstruction(Chunk* chunk, int offset);
    static int simpleInstruction(const char* name, int offset);
    static int constantInstruction(const char* name, Chunk* chunk, int offset);
};