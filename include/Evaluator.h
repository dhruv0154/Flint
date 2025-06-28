#pragma once

#include "C:\Flint\include\Scanner\generated\Expr.h"

struct Evaluator 
{
    LiteralValue operator()(const Binary& expr) const 
    {
        LiteralValue left = evaluate(*expr.left);
        LiteralValue right = evaluate(*expr.right);

        if (expr.op.type == TokenType::PLUS)
            return std::get<double>(left) + std::get<double>(right);
        else if (expr.op.type == TokenType::MINUS)
            return std::get<double>(left) - std::get<double>(right);
        else if (expr.op.type == TokenType::STAR)
            return std::get<double>(left) * std::get<double>(right);
        else if (expr.op.type == TokenType::SLASH)
            return std::get<double>(left) / std::get<double>(right);
        return std::monostate{};
    }

    LiteralValue operator()(const Unary& expr) const 
    {
        LiteralValue right = evaluate(*expr.right);
        if (expr.op.type == TokenType::MINUS)
            return -std::get<double>(right);
        if (expr.op.type == TokenType::BANG)
            return !std::get<bool>(right);
        return std::monostate{};
    }

    LiteralValue operator()(const Literal& expr) const {
        return expr.value;
    }

    LiteralValue operator()(const Grouping& expr) const {
        return evaluate(*expr.expression);
    }

    LiteralValue evaluate(const ExpressionNode& expr) const {
        return std::visit(*this, expr);
    }
};