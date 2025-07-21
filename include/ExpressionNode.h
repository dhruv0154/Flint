#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  ExpressionNode System (AST Nodes for Expressions)
// ─────────────────────────────────────────────────────────────────────────────
//  This file defines the core expression types in the Flint language.
//
//  Expressions include literals, binary ops, conditionals, variables, etc.
//  Each node is represented by a struct and wrapped in a std::variant.
//  The `ExprPtr` alias wraps nodes with std::shared_ptr for polymorphic access.
//
//  Example Expression:
//      let x = (true ? 10 + 2 : 5);
//      Would be represented as nested Conditional, Binary, Literal, etc.
// ─────────────────────────────────────────────────────────────────────────────

#include <memory>
#include <vector>
#include <string>
#include <variant>
#include <any>
#include "Scanner/Token.h"
#include "Scanner/Value.h"

// ─────────────────────────────────────────────────────────────
// Forward declarations of statement structs
// ─────────────────────────────────────────────────────────────
struct ExpressionStmt;
struct FunctionStmt;
struct IfStmt;
struct WhileStmt;
struct ReturnStmt;
struct BreakStmt;
struct ContinueStmt;
struct TryCatchContinueStmt;
struct LetStmt;
struct BlockStmt;
struct ClassStmt;

// ─────────────────────────────────────────────────────────────
// Statement Variant
// ─────────────────────────────────────────────────────────────
// Acts as the base type for all statements in the AST.
// Used in: Parser output and Interpreter execution
// ─────────────────────────────────────────────────────────────
using Statement = std::variant<
    ExpressionStmt,
    FunctionStmt, 
    WhileStmt,
    ReturnStmt,
    BreakStmt,
    ContinueStmt,
    TryCatchContinueStmt,
    IfStmt, 
    LetStmt, 
    BlockStmt,
    ClassStmt
>;

// ─────────────────────────────────────────────────────────────
// Forward declarations for all supported expression types
// ─────────────────────────────────────────────────────────────
struct Binary;
struct Logical;
struct Call;
struct Unary;
struct Literal;
struct Grouping;
struct Conditional;
struct Variable;
struct Assignment;
struct Lambda;
struct Get;
struct Set;
struct This;

// ─────────────────────────────────────────────────────────────
// ExpressionNode variant: acts like a base class for AST nodes
// ─────────────────────────────────────────────────────────────
using ExpressionNode = std::variant<
    Binary,
    Call,
    Logical,
    Unary,
    Literal,
    Grouping,
    Conditional,
    Variable,
    Assignment,
    Lambda,
    Get,
    Set,
    This
>;

// Smart pointer for expression nodes
using ExprPtr = std::shared_ptr<ExpressionNode>;

// ─────────────────────────────────────────────────────────────
// Binary: (left op right)
// ─────────────────────────────────────────────────────────────
struct Binary 
{
    ExprPtr left;   // Left-hand side expression
    Token op;       // Operator token (e.g., +, -, *, ==)
    ExprPtr right;  // Right-hand side expression

    Binary(ExprPtr left, Token op, ExprPtr right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
};

struct Logical
{
    ExprPtr left;
    Token op;
    ExprPtr right;

    Logical(ExprPtr left, Token op, ExprPtr right)
            : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
};

// ─────────────────────────────────────────────────────────────
// Conditional: (condition ? left : right)
// ─────────────────────────────────────────────────────────────
struct Conditional 
{
    ExprPtr condition; // Ternary condition (before ?)
    ExprPtr left;      // Expression if true
    ExprPtr right;     // Expression if false

    Conditional(ExprPtr condition, ExprPtr left, ExprPtr right)
        : condition(std::move(condition)), left(std::move(left)), right(std::move(right)) {}
};

struct Call
{
    ExprPtr callee;
    Token paren;
    std::vector<ExprPtr> arguments;

    Call(ExprPtr callee, Token paren, std::vector<ExprPtr> arguments) :
        callee(std::move(callee)), paren(paren), arguments(arguments) {}
};

// ─────────────────────────────────────────────────────────────
// Unary: (op right)
// ─────────────────────────────────────────────────────────────
struct Unary 
{
    Token op;       // Unary operator token (e.g., !, -)
    ExprPtr right;  // Operand

    Unary(Token op, ExprPtr right)
        : op(std::move(op)), right(std::move(right)) {}
};

// ─────────────────────────────────────────────────────────────
// Literal: (e.g., 42, "hello", true)
// ─────────────────────────────────────────────────────────────
struct Literal 
{
    LiteralValue value;  // Can be string, number, bool, etc.

    Literal(LiteralValue value)
        : value(std::move(value)) {}
};

// ─────────────────────────────────────────────────────────────
// Grouping: ((expr))
// ─────────────────────────────────────────────────────────────
struct Grouping 
{
    ExprPtr expression;  // Nested expression in parentheses

    Grouping(ExprPtr expression)
        : expression(std::move(expression)) {}
};

// ─────────────────────────────────────────────────────────────
// Variable: (identifier usage)
// ─────────────────────────────────────────────────────────────
struct Variable
{
    Token name;  // Token representing the variable name

    Variable(Token name)
        : name(name) {}
};

struct Assignment
{
    Token name;
    ExprPtr value;

    Assignment(Token name, ExprPtr value)
        : name(name), value(std::move(value)) {}
};

struct Lambda
{
    std::shared_ptr<FunctionStmt> function;
    Lambda(std::shared_ptr<FunctionStmt> function)
        : function(std::move(function)) {}
};

struct Get
{
    ExprPtr object;
    Token name;

    Get(ExprPtr object, Token name) : object(std::move(object)), name(name) {}
};

struct Set
{
    ExprPtr object;
    Token name;
    ExprPtr value;

    Set(ExprPtr object, Token name, ExprPtr value) : object(std::move(object)), 
        name(name), value(std::move(value)) {}
};

struct This
{
    Token keyword;

    This(Token keyword) : keyword(keyword) {}
};