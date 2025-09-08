#include <iostream>
#include <functional>
#include "common.h"
#include "vm.h"
#include "debug.h"

template<typename Op>
inline void VM::binaryOp(Op op) {
    double b = stack.pop();
    double a = stack.pop();
    stack.push_back(op(a, b));
}

InterpretResult VM::interpret(const std::string source)
{
    compile(source);
    return InterpretResult::INTERPRET_OK;
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
        std::cout << "    ";
        for(auto slot = stack.getData(); slot < stack.getData() + stack.size(); slot++)
        {
            std::cout << "[ ";
            std::cout << *slot;
            std::cout << " ]";
        }
        std::cout << std::endl;
        disassembler.disassembleInstruction(chunk, (int)(ip -  chunk -> getCode().getData()));
        #endif
        uint8_t instruction = readByte();
        OpCode instructionCode = static_cast<OpCode>(instruction);
        switch(instructionCode)
        {
            case OpCode::OP_CONSTANT: {
                Value constant = readConstant();
                stack.push_back(constant);
                break;
            }
            case OpCode::OP_RETURN: {
                std::cout << stack.pop();
                std::cout << "\n";
                return InterpretResult::INTERPRET_OK;
            }

            case OpCode::OP_NEGATE: stack[stack.size() - 1] = -stack[stack.size() - 1]; break;
            case OpCode::OP_ADD: binaryOp(std::plus<>()); break;
            case OpCode::OP_SUBTRACT: binaryOp(std::minus<>()); break;
            case OpCode::OP_MULTIPLY: binaryOp(std::multiplies<>()); break;
            case OpCode::OP_DIVIDE: binaryOp(std::divides<>()); break;


        }
    }
}