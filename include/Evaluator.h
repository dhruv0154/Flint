#pragma once

#include <memory>
#include "ExpressionNode.h"

class Evaluator 
{
public:
    LiteralValue operator()(const Binary& expr) const;
    LiteralValue operator()(const Conditional& expr) const;
    LiteralValue operator()(const Unary& expr) const;
    LiteralValue operator()(const Literal& expr) const;
    LiteralValue operator()(const Grouping& expr) const;

    LiteralValue evaluate(const ExpressionNode& expr) const;
    LiteralValue evaluate(const std::shared_ptr<ExpressionNode>& ptr) const;

    bool isTruthy(const LiteralValue& value) const;
    bool isEqual(const LiteralValue& left, const LiteralValue& right) const;
    template<typename... Operands>
    void checkOperandType(const Token& op, const Operands&... operands) const;
};
