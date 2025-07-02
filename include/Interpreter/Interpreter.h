#pragma once

#include "ExpressionNode.h"
#include "Evaluator.h"

class Interpreter
{
public:
    static std::string stringify(const LiteralValue& val);
    void interpret(std::variant<Binary, Unary, Literal, Grouping, Conditional> expr) const;
};