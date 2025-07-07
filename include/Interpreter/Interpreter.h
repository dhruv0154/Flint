#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  Interpreter Class
// ─────────────────────────────────────────────────────────────────────────────
//  The `Interpreter` is the runtime engine of the Flint language.
//  It evaluates full programs by executing statements (not expressions).
//
//  Responsibilities:
//  - Executes statements: print, let, expression
//  - Coordinates evaluation by delegating to `Evaluator`
//  - Manages runtime environment via `Environment`
//
//  This class implements the visitor pattern over statements (PrintStmt, etc.).
// ─────────────────────────────────────────────────────────────────────────────

#include "ExpressionNode.h"
#include "Flint/Environment.h"
#include "Evaluator.h"
#include "Stmt.h"

class Interpreter
{
public:
    // ─────────────────────────────────────────────────────────────
    // Global singletons shared across execution
    // ─────────────────────────────────────────────────────────────

    // Evaluator: used to evaluate expression nodes recursively
    static std::unique_ptr<Evaluator> evaluator;

    // Environment: stores variable bindings (let x = ...)
    static std::unique_ptr<Environment> environment;

    // ─────────────────────────────────────────────────────────────
    // Statement Visitors
    // These operator() overloads are invoked by std::visit() on statements.
    // ─────────────────────────────────────────────────────────────

    // Handles `print` statements: evaluates and outputs value
    void operator()(const PrintStmt& stmt) const;

    // Handles standalone expressions (e.g., `a + b;`)
    void operator()(const ExpressionStmt& exprStatement) const;

    // Handles variable declarations (`let x = 42;`)
    void operator()(const LetStmt& letStatement) const;

    // ─────────────────────────────────────────────────────────────
    // Runtime Helpers
    // ─────────────────────────────────────────────────────────────

    // Converts any LiteralValue (double, bool, nil, string) to a string.
    // Used for printing and debug output.
    static std::string stringify(const LiteralValue& val);

    // ─────────────────────────────────────────────────────────────
    // Entry Points
    // ─────────────────────────────────────────────────────────────

    // Executes a list of statements (used for full programs or REPL lines)
    void interpret(std::vector<std::shared_ptr<Statement>> statements) const;

    // Executes a single statement (used internally for each stmt in `interpret`)
    void execute(std::shared_ptr<Statement> statement) const;
};