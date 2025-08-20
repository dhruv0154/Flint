#include <iostream>
#include "vm.h"

InterpretResult VM::interpret(Chunk* chunk)
{
    this -> chunk = chunk;
    ip = this -> chunk -> getCode();
    return run();
}

InterpretResult VM::run()
{
    auto readByte = [&]() -> uint8_t { return *ip++; };
    auto readConstant = [&]() -> Value { return 
        chunk -> getConstants().getValues()[readByte()]; };
    for(;;)
    {
        uint8_t instruction;
        switch(instruction = readByte())
        {
            case static_cast<uint8_t>(OpCode::OP_CONSTANT): {
                Value constant = readConstant();
                ValueArray::printValue(constant);
                std::cout << "\n";
                break;
            }
            case static_cast<uint8_t>(OpCode::OP_RETURN): {
                return InterpretResult::INTERPRET_OK;
            }
        }
    }
}