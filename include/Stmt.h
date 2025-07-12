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
struct FunctionStmt;
struct IfStmt;
struct WhileStmt;
struct ReturnStmt;
struct BreakStmt;
struct ContinueStmt;
struct TryCatchContinueStmt;
struct LetStmt;
struct BlockStmt;

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
    BlockStmt
>;

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


struct IfStmt
{
    ExprPtr condition;
    std::shared_ptr<Statement> thenBranch;
    std::shared_ptr<Statement> elseBranch;

    IfStmt(ExprPtr condition, 
        std::shared_ptr<Statement> thenBranch, std::shared_ptr<Statement> elseBranch) :
        condition(condition), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {}
};

struct FunctionStmt
{
    Token name;
    std::vector<Token> params;
    std::vector<std::shared_ptr<Statement>> body;

    FunctionStmt(Token name, std::vector<Token> params, std::vector<std::shared_ptr<Statement>> body) :
        name(name), params(params), body(body) {}
};

struct WhileStmt
{
    ExprPtr condition;
    std::shared_ptr<Statement> statement;

    WhileStmt(ExprPtr condition, std::shared_ptr<Statement> statement) :
        condition(std::move(condition)), statement(std::move(statement)) {}
};

struct ReturnStmt
{
    Token keyword;
    ExprPtr val;

    ReturnStmt(Token keyword, ExprPtr val) : keyword(keyword), val(val) {}
};

struct BreakStmt
{
    Token keyword;

    BreakStmt(Token keyword) : keyword(keyword) {}
};

struct ContinueStmt
{
    Token keyword;

    ContinueStmt(Token keyword) : keyword(keyword) {}
};

struct TryCatchContinueStmt
{
    std::shared_ptr<Statement> body;

    TryCatchContinueStmt(std::shared_ptr<Statement> body) : body(body) {}
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

struct BlockStmt
{
    std::vector<std::shared_ptr<Statement>> statements;

    BlockStmt(std::vector<std::shared_ptr<Statement>> statements) :
        statements(statements) {}
};