#pragma once

#include <unordered_map>
#include "Flint/ASTNodes/Stmt.h"
#include "Flint/ASTNodes/ExpressionNode.h"
#include "Flint/Callables/Functions/FunctionType.h"
#include "Flint/Callables/Classes/ClassType.h"

class Interpreter;

// Responsible for performing static resolution of variable scopes and bindings
class Resolver
{
private:
    std::shared_ptr<Interpreter> interpreter; // Reference to the interpreter to inform about resolved variables
    std::vector<std::unordered_map<std::string, bool>> scopes; // Stack of scopes for variables, each map holds variable declarations in the current scope
    FunctionType currentFunction; // Tracks the current function type to detect invalid returns or recursion
    ClassType currentClass; // Tracks the current class context to validate 'this' and methods

public:

    // === Statement Resolvers ===

    void operator()(const WhileStmt& stmt);                    // Resolves loop condition and body
    void operator()(const FunctionStmt& stmt);                 // Resolves function declarations and their bodies
    void operator()(const ReturnStmt& stmt);                   // Validates and resolves return expressions
    void operator()(const IfStmt& stmt);                       // Resolves condition, then, and else blocks
    void operator()(const BreakStmt& stmt);                    // Validates break context (e.g., inside loops)
    void operator()(const ContinueStmt& stmt);                 // Validates continue context
    void operator()(const TryCatchContinueStmt& stmt);         // Resolves try-catch with custom "continue" recovery

    void operator()(const ExpressionStmt& exprStatement);      // Resolves simple expression statements
    void operator()(const LetStmt& letStatement);              // Handles variable declaration and optional initializer
    void operator()(const BlockStmt& blockStatement);          // Resolves a scoped block of statements
    void operator()(const ClassStmt& classStatement);          // Resolves class declarations and method scopes

    // === Expression Resolvers ===

    void operator()(const Binary& expr);                       // Resolves binary operators
    void operator()(const Logical& expr);                      // Resolves logical operations (AND/OR)
    void operator()(const Conditional& expr);                  // Resolves ternary (?:) conditions
    void operator()(const Unary& expr);                        // Resolves unary operations
    void operator()(const Literal& expr);                      // Literals (no resolution needed)
    void operator()(const Grouping& expr);                     // Resolves grouped sub-expressions
    void operator()(const Variable& expr, ExprPtr exprPtr);    // Resolves usage of a variable
    void operator()(const Assignment& expr, ExprPtr exprPtr);  // Resolves assignment targets and values
    void operator()(const Lambda& expr);                       // Resolves lambda (anonymous) function expression
    void operator()(const Call& expr);                         // Resolves function/method calls
    void operator()(const Get& expr);                          // Resolves property access (obj.prop)
    void operator()(const Set& expr);                          // Resolves property assignments (obj.prop = value)
    void operator()(const This& expr, ExprPtr exprPtr);        // Resolves 'this' keyword inside classes
    void operator()(const Super& expr, ExprPtr exprPtr);       // Resolves 'super' keyword inside classes
    void operator()(const Array& expr);
    void operator()(const GetIndex& expr);
    void operator()(const SetIndex& expr);

    // === Entry Points for Resolution ===

    void resolve(std::vector<std::shared_ptr<Statement>> statements); // Entry for resolving a list of statements
    void resolve(std::shared_ptr<Statement> stmt);                    // Entry for resolving a single statement
    void resolve(ExprPtr expr);                                       // Entry for resolving a single expression
    void resolveLocal(ExprPtr expr, Token name);                      // Resolve a variable in the current/local scope
    void resolveFunction(const FunctionStmt &stmt, FunctionType type);// Handle function-specific resolution context

    // === Scope Management ===

    void beginScope();  // Begins a new local scope
    void endScope();    // Ends the current local scope

    void declare(Token name); // Declares a variable (name only, before value)
    void define(Token name);  // Defines a variable (after its value has been resolved)

    // === Constructor ===

    Resolver(std::shared_ptr<Interpreter> interpreter) 
        : interpreter(interpreter), currentFunction(FunctionType::NONE), 
          currentClass(ClassType::NONE) {}
};