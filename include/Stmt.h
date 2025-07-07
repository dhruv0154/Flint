#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  Stmt.h – AST Nodes for Statements in Flint
// ─────────────────────────────────────────────────────────────────────────────
//  Defines all types of executable statements (non-expressions).
//
//  In Flint, a program is a list of statements, not just expressions.
//
//  Supported statement types:
//   - ExpressionStmt : evaluates an expression (e.g., a + b;)
//   - PrintStmt      : outputs a value (e.g., print x;)
//   - LetStmt        : variable declaration (e.g., let x = 5;)
//
//
//  Statement is a tagged union (`std::variant`) that wraps all supported
//  statement types. They are executed by the Interpreter via pattern matching.
// ─────────────────────────────────────────────────────────────────────────────

#include <memory>
#include <variant>
#include "ExpressionNode.h"  // Defines ExprPtr

// ─────────────────────────────────────────────────────────────
// Forward declarations of statement structs
// ─────────────────────────────────────────────────────────────
struct ExpressionStmt;
struct PrintStmt;
struct LetStmt;

// ─────────────────────────────────────────────────────────────
// Statement Variant
// ─────────────────────────────────────────────────────────────
// Acts as the base type for all statements in the AST.
// Used in: Parser output and Interpreter execution
// ─────────────────────────────────────────────────────────────
using Statement = std::variant<ExpressionStmt, PrintStmt, LetStmt>;

// ─────────────────────────────────────────────────────────────
// Expression Statement
// ─────────────────────────────────────────────────────────────
// Example: `x + 1;`
// Used for evaluating expressions without storing or printing them.
// ─────────────────────────────────────────────────────────────
struct ExpressionStmt 
{
    ExprPtr expression;  // Any expression (e.g., Binary, Grouping)

    ExpressionStmt(ExprPtr expr)
        : expression(std::move(expr)) {}
};

// ─────────────────────────────────────────────────────────────
// Print Statement
// ─────────────────────────────────────────────────────────────
// Example: `print x + 1;`
// Used to output a value (handled by Interpreter).
// ─────────────────────────────────────────────────────────────
struct PrintStmt 
{
    ExprPtr expression;  // Expression whose result is printed

    PrintStmt(ExprPtr expr)
        : expression(std::move(expr)) {}
};

// ─────────────────────────────────────────────────────────────
// Let Statement (Variable Declaration)
// ─────────────────────────────────────────────────────────────
// Example: `let x = 42;`
// Introduces a new variable with a name and initial value.
// ─────────────────────────────────────────────────────────────
struct LetStmt
{
    Token name;          // Variable name token
    ExprPtr expression;  // Initializing expression

    LetStmt(Token name, ExprPtr expr) 
        : name(name), expression(std::move(expr)) {}
};