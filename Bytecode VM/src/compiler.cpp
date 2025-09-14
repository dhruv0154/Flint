#include <string>
#include <iostream>
#include <iomanip>
#include "common.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

#include "compiler.h"
#include "scanner.h"
#include "chunk.h"

bool Compiler::compile(const std::string &src, Chunk* chunk)
{
    Scanner scanner(src);
    parser.scanner = scanner;
    compilingChunk = chunk;
    parser.advance();
    parser.expression();
    parser.consume(TokenType::END_OF_FILE, "Expected eof expression at the end."); 
    endCompiler();
    return !parser.hadError;
}

int Compiler::makeConstant(Value value) 
{
    int constant = compilingChunk -> addConstant(value);
    if (constant > 0xFFFFFF) 
    {
        parser.error("Too many constants in one chunk.");
        return 0;
    }
    return constant;
}

void Compiler::emitConstant(Value value)
{
    uint8_t constant = makeConstant(value);
    if (constant <= UINT8_MAX) 
    {
        emitBytes(static_cast<uint8_t>(OpCode::OP_CONSTANT), (uint8_t)constant);
    } 
    else 
    {
        emitByte(static_cast<uint8_t>(OpCode::OP_CONSTANT_LONG));

        // write as 3 bytes, little-endian
        emitByte(constant & 0xFF);
        emitByte((constant >> 8) & 0xFF);
        emitByte((constant >> 16) & 0xFF);
    }
}

void Compiler::emitByte(uint8_t byte)
{
    compilingChunk -> writeChunk(byte, parser.previous.line);
}

void Compiler::emitBytes(uint8_t byte1, uint8_t byte2)
{
    emitByte(byte1);
    emitByte(byte2);
}

void Compiler::endCompiler()
{
    emitReturn();
    #ifdef DEBUG_PRINT_CODE
        if (!parser.hadError) {
            Disassembler::disassembleChunk(compilingChunk, "code");
        }
    #endif
}

void Compiler::emitReturn()
{
    emitByte(static_cast<uint8_t>(OpCode::OP_RETURN));
}