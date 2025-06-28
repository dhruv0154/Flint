#pragma once

#include <memory>
#include <vector>
#include <string>
#include <variant>
#include <any>
#include "C:\Flint\include\Scanner\Token.h"
#include <C:\Flint\include\Scanner\Value.h>

// Forward declarations
struct Binary;
struct Unary;
struct Literal;
struct Grouping;

using ExpressionNode = std::variant<Binary, Unary, Literal, Grouping>;
using ExprPtr = std::shared_ptr<ExpressionNode>;

struct Binary {
    ExprPtr left;
    Token op;
    ExprPtr right;
};

struct Unary {
    Token op;
    ExprPtr right;
};

struct Literal {
    LiteralValue value;

    Literal() = default;
    Literal(LiteralValue v) : value(v) {}
};

struct Grouping {
    ExprPtr expression;
};
