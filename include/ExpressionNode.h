#pragma once

#include <memory>
#include <vector>
#include <string>
#include <variant>
#include <any>
#include "Scanner/Token.h"
#include "Scanner/Value.h"

// Forward declarations
struct Binary;
struct Unary;
struct Literal;
struct Grouping;
struct Conditional;

using ExpressionNode = std::variant<Binary, Unary, Literal, Grouping, Conditional>;
using ExprPtr = std::shared_ptr<ExpressionNode>;

struct Binary {
    ExprPtr left;
    Token op;
    ExprPtr right;

    Binary(ExprPtr left, Token op, ExprPtr right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
};

struct Conditional {
    ExprPtr condition;
    ExprPtr left;
    ExprPtr right;

    Conditional(ExprPtr condition, ExprPtr left, ExprPtr right)
        : condition(std::move(condition)), left(std::move(left)), right(std::move(right)) {}
};

struct Unary {
    Token op;
    ExprPtr right;

    Unary(Token op, ExprPtr right)
        : op(std::move(op)), right(std::move(right)) {}
};

struct Literal {
    LiteralValue value;

    Literal(LiteralValue value)
        : value(std::move(value)) {}
};

struct Grouping {
    ExprPtr expression;

    Grouping(ExprPtr expression)
        : expression(std::move(expression)) {}
};