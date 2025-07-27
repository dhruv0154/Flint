#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  Stmt.h – AST Nodes for Statements in Flint
// ─────────────────────────────────────────────────────────────────────────────
//  Defines every executable statement in Flint.  A program is a list of
//  Statement nodes, parsed from source and executed by the Interpreter.
//
//  Supported statements include variable declarations, control flow,
//  function and class definitions, and more.
// ─────────────────────────────────────────────────────────────────────────────

#include <memory>
#include <variant>
#include <optional>
#include "ExpressionNode.h"  // Provides ExprPtr for embedding expressions

// ─────────────────────────────────────────────────────────────
//  Forward declarations of statement structs
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
//  Statement
// ─────────────────────────────────────────────────────────────
//  Variant wrapping all statement node types.  Used by the parser
//  to build AST and by the interpreter to execute via std::visit.
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
//  ExpressionStmt
// ─────────────────────────────────────────────────────────────
//  Wraps an expression as a statement to evaluate it for side effects
//  Example: `x + 1;` discards the result but may trigger errors or calls
struct ExpressionStmt {
    ExprPtr expression;  // Expression to evaluate

    ExpressionStmt(ExprPtr expr)
        : expression(std::move(expr)) {}
};

// ─────────────────────────────────────────────────────────────
//  IfStmt
// ─────────────────────────────────────────────────────────────
//  Conditional execution of thenBranch or elseBranch based on condition.
struct IfStmt {
    ExprPtr condition;                        // Boolean expression
    std::shared_ptr<Statement> thenBranch;    // Executed when true
    std::shared_ptr<Statement> elseBranch;    // Executed when false (optional)

    IfStmt(ExprPtr condition,
           std::shared_ptr<Statement> thenBranch,
           std::shared_ptr<Statement> elseBranch)
        : condition(std::move(condition)),
          thenBranch(std::move(thenBranch)),
          elseBranch(std::move(elseBranch)) {}
};

// ─────────────────────────────────────────────────────────────
//  FunctionStmt
// ─────────────────────────────────────────────────────────────
//  Declares a (possibly anonymous) function or getter in the current scope.
//  If isGetter==true, the function is invoked as a property getter.
struct FunctionStmt {
    std::optional<Token> name;                 // Function name; empty for lambdas
    std::vector<Token> params;                 // Parameter names
    std::vector<std::shared_ptr<Statement>> body; // Statements in function body
    bool isGetter;                             // Marks getter methods

    FunctionStmt(std::optional<Token> name,
                 std::vector<Token> params,
                 std::vector<std::shared_ptr<Statement>> body,
                 bool isGetter = false)
        : name(std::move(name)),
          params(std::move(params)),
          body(std::move(body)),
          isGetter(isGetter) {}
};

// ─────────────────────────────────────────────────────────────
//  WhileStmt
// ─────────────────────────────────────────────────────────────
//  Repeatedly executes statement as long as condition is true.
struct WhileStmt {
    ExprPtr condition;                      // Loop condition expression
    std::shared_ptr<Statement> statement;   // Body to execute each iteration

    WhileStmt(ExprPtr condition,
              std::shared_ptr<Statement> statement)
        : condition(std::move(condition)),
          statement(std::move(statement)) {}
};

// ─────────────────────────────────────────────────────────────
//  ReturnStmt
// ─────────────────────────────────────────────────────────────
//  Exits a function, optionally returning a value.
struct ReturnStmt {
    Token keyword;  // 'return' token, used for error reporting
    ExprPtr val;    // Expression whose value is returned

    ReturnStmt(Token keyword, ExprPtr val)
        : keyword(std::move(keyword)), val(std::move(val)) {}
};

// ─────────────────────────────────────────────────────────────
//  BreakStmt & ContinueStmt
// ─────────────────────────────────────────────────────────────
//  Alters control flow of loops.  Break exits, continue skips to next
struct BreakStmt {
    Token keyword;
    BreakStmt(Token keyword) : keyword(std::move(keyword)) {}
};

struct ContinueStmt {
    Token keyword;
    ContinueStmt(Token keyword) : keyword(std::move(keyword)) {}
};

// ─────────────────────────────────────────────────────────────
//  TryCatchContinueStmt
// ─────────────────────────────────────────────────────────────
//  Used internally for desugaring 'for' loops: wraps loop body
//  so that 'continue' can advance the loop index before repeating.
struct TryCatchContinueStmt {
    std::shared_ptr<Statement> body;  // Original loop body

    TryCatchContinueStmt(std::shared_ptr<Statement> body)
        : body(std::move(body)) {}
};

// ─────────────────────────────────────────────────────────────
//  LetStmt
// ─────────────────────────────────────────────────────────────
//  Declares one or more variables in the current environment.
//  Example: `let x = 5, y = 10;`
struct LetStmt {
    std::vector<std::pair<Token, ExprPtr>> declarations;

    LetStmt(std::vector<std::pair<Token, ExprPtr>> declarations)
        : declarations(std::move(declarations)) {}
};

// ─────────────────────────────────────────────────────────────
//  BlockStmt
// ─────────────────────────────────────────────────────────────
//  A sequence of statements with its own scope.
struct BlockStmt {
    std::vector<std::shared_ptr<Statement>> statements;

    BlockStmt(std::vector<std::shared_ptr<Statement>> statements)
        : statements(std::move(statements)) {}
};

// ─────────────────────────────────────────────────────────────
//  ClassStmt
// ─────────────────────────────────────────────────────────────
//  Declares a class with instance and static (class) methods.
struct ClassStmt {
    Token name;  // Class identifier
    ExprPtr superClass;
    std::vector<std::shared_ptr<Statement>> instanceMethods; // Methods on instances
    std::vector<std::shared_ptr<Statement>> classMethods;    // Static methods

    ClassStmt(Token name,
              ExprPtr superClass,
              std::vector<std::shared_ptr<Statement>> instanceMethods,
              std::vector<std::shared_ptr<Statement>> classMethods)
        : name(std::move(name)),
          superClass(std::move(superClass)),
          instanceMethods(std::move(instanceMethods)),
          classMethods(std::move(classMethods)) {}
};