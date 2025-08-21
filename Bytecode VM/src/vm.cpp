#include <iostream>
#include "common.h"
#include "vm.h"
#include "debug.h"

InterpretResult VM::interpret(Chunk* chunk)
{
    this -> chunk = chunk;
    ip = this -> chunk -> getCode().getData();
    return run();
}

InterpretResult VM::run()
{
    auto readByte = [&]() -> uint8_t { return *ip++; };
    Disassembler disassembler;
    auto readConstant = [&]() -> Value { return 
        chunk -> getConstants()[readByte()]; };
    for(;;)
    {
        #ifdef DEBUG_TRACE_EXECUTION
        disassembler.disassembleInstruction(chunk, (int)(ip -  chunk -> getCode().getData()));
        #endif
        uint8_t instruction;
        switch(instruction = readByte())
        {
            case static_cast<uint8_t>(OpCode::OP_CONSTANT): {
                Value constant = readConstant();
                std::cout << constant;
                std::cout << "\n";
                break;
            }
            case static_cast<uint8_t>(OpCode::OP_RETURN): {
                return InterpretResult::INTERPRET_OK;
            }
        }
    }
}