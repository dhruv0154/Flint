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
private:
    mutable bool isInsideLoop = false;
    std::shared_ptr<Environment> globals;
    std::unordered_map<ExprPtr, int> locals;
        // Evaluator: used to evaluate expression nodes recursively
    mutable std::unique_ptr<Evaluator> evaluator;

    // Environment: stores variable bindings (let x = ...)
    mutable std::shared_ptr<Environment> environment;

public:
    // ─────────────────────────────────────────────────────────────
    // Global singletons shared across execution
    // ─────────────────────────────────────────────────────────────




    // ─────────────────────────────────────────────────────────────
    // Statement Visitors
    // These operator() overloads are invoked by std::visit() on statements.
    // ─────────────────────────────────────────────────────────────

    void operator()(const WhileStmt& stmt) const;

    void operator()(const FunctionStmt& stmt) const;

    void operator()(const ReturnStmt& stmt) const;

    void operator()(const IfStmt& stmt) const;

    void operator()(const BreakStmt& stmt) const;

    void operator()(const ContinueStmt& stmt) const;

    void operator()(const TryCatchContinueStmt& stmt) const;

    // Handles standalone expressions (e.g., `a + b;`)
    void operator()(const ExpressionStmt& exprStatement) const;

    // Handles variable declarations (`let x = 42;`)
    void operator()(const LetStmt& letStatement) const;

    // Handles block statments ({ 'Collection of statements' })
    void operator()(const BlockStmt& blockStatement) const;


    // ─────────────────────────────────────────────────────────────
    // Runtime Helpers
    // ─────────────────────────────────────────────────────────────

    bool isNumber(const std::string& str);
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

    // Executes a block statements (used internally for each stmt in `blockVisitor`)
    void executeBlock(std::vector<std::shared_ptr<Statement>> statements, 
            std::shared_ptr<Environment> newEnv) const;
    
    void resolve(ExprPtr expr, int depth);

    friend class Evaluator;
    Interpreter();
};