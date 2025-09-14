#pragma once

#include "common.h"
#include "chunk.h"
#include "parser.h"
#include <string>

class Compiler
{
private:
    Chunk* compilingChunk;
    Parser parser;
    void endCompiler();
    int makeConstant(Value value);
    void emitReturn();
public:
    void emitByte(uint8_t byte);
    void emitBytes(uint8_t byte1, uint8_t byte2);
    void emitConstant(Value value);
    Compiler() : parser(this) {}
    bool compile(const std::string &src, Chunk* chunk);
    ~Compiler() = default;
};