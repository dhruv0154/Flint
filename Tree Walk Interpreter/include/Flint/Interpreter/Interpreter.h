#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  Interpreter.h – Runtime Statement Execution for Flint
// ─────────────────────────────────────────────────────────────────────────────
//  Defines the Interpreter class, which executes statement AST nodes
//  and orchestrates expression evaluation via the Evaluator.
//  Manages the runtime environment (variable scopes, function contexts).
// ─────────────────────────────────────────────────────────────────────────────

#include <unordered_map>
#include <memory>
#include <vector>
#include <string>
#include "Flint/ASTNodes/ExpressionNode.h"       // AST nodes for expressions
#include "Flint/Environment.h"    // Environment for variable scopes
#include "Evaluator.h"            // Expression evaluator
#include "Flint/ASTNodes/Stmt.h"                 // AST nodes for statements

class Interpreter {
private:
    //──────────────────────────────────────────────────────────────────────────
    // isInsideLoop: flag to track if currently in a loop context
    // Used to validate break/continue usage and manage loop control flow.
    //──────────────────────────────────────────────────────────────────────────
    mutable bool isInsideLoop = false;

    //──────────────────────────────────────────────────────────────────────────
    // globals: the global (outermost) environment
    // Stores all top-level variable and function definitions.
    //──────────────────────────────────────────────────────────────────────────
    std::shared_ptr<Environment> globals;

    //──────────────────────────────────────────────────────────────────────────
    // locals: mapping from expression nodes to their lexical scope depth
    // Populated by the Resolver.  Enables fast lookups via Environment.getAt().
    //──────────────────────────────────────────────────────────────────────────
    std::unordered_map<ExprPtr, int> locals;

    //──────────────────────────────────────────────────────────────────────────
    // evaluator: helper object to compute expression values
    // Created on first use; uses this interpreter for context (e.g., function calls).
    //──────────────────────────────────────────────────────────────────────────
    mutable std::unique_ptr<Evaluator> evaluator;

    //──────────────────────────────────────────────────────────────────────────
    // environment: current environment (changes in blocks, functions)
    // Points to globals initially, then to nested block/function scopes.
    //──────────────────────────────────────────────────────────────────────────
    mutable std::shared_ptr<Environment> environment;

public:
    //──────────────────────────────────────────────────────────────────────────
    // Statement Visitors: executes different statement types
    // Invoked by std::visit on Statement variant.
    //──────────────────────────────────────────────────────────────────────────

    // while (condition) body
    void operator()(const WhileStmt& stmt) const;

    // Function declaration: registers function in current environment
    void operator()(const FunctionStmt& stmt) const;

    // return statement inside function
    void operator()(const ReturnStmt& stmt) const;

    // if (condition) thenBranch else elseBranch
    void operator()(const IfStmt& stmt) const;

    // break: exit current loop
    void operator()(const BreakStmt& stmt) const;

    // continue: jump to next loop iteration
    void operator()(const ContinueStmt& stmt) const;

    // internal node for for-loop desugaring: ensures continue advances index
    void operator()(const TryCatchContinueStmt& stmt) const;

    // expression stmt: evaluate and discard result
    void operator()(const ExpressionStmt& exprStatement) const;

    // let stmt: declare variables
    void operator()(const LetStmt& letStatement) const;

    // block stmt: execute a series of statements in new scope
    void operator()(const BlockStmt& blockStatement) const;

    // class declaration: define a new class
    void operator()(const ClassStmt& classStmt) const;

    //──────────────────────────────────────────────────────────────────────────
    // Runtime Helpers
    //──────────────────────────────────────────────────────────────────────────

    // Checks if a string represents a valid number literal (for parsing inputs)
    bool isNumber(const std::string& str);

    // Converts any LiteralValue to its string representation
    // Used by print and error messages.
    static std::string stringify(const LiteralValue& val);

    //──────────────────────────────────────────────────────────────────────────
    // Entry Points for Execution
    //──────────────────────────────────────────────────────────────────────────

    // Execute a list of statement nodes (program or REPL line)
    void interpret(std::vector<std::shared_ptr<Statement>> statements) const;

    // Execute a single statement
    void execute(std::shared_ptr<Statement> statement) const;

    // Execute a block of statements in a new environment
    void executeBlock(std::vector<std::shared_ptr<Statement>> statements,
                      std::shared_ptr<Environment> newEnv) const;

    // Store resolved scope depth for a variable expression
    void resolve(ExprPtr expr, int depth);

    // Allow Evaluator to access private members (environment, locals)
    friend class Evaluator;

    // Constructor: initializes global environment and evaluator
    Interpreter();
};