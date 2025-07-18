#pragma once

#include <unordered_map>
#include "Stmt.h"
#include "ExpressionNode.h"
#include "FunctionType.h"

class Interpreter;

class Resolver
{
private:
    std::shared_ptr<Interpreter> interpreter;
    std::vector<std::unordered_map<std::string, bool>> scopes;
    FunctionType currentFunction;
public:

    void operator()(const WhileStmt& stmt);

    void operator()(const FunctionStmt& stmt);

    void operator()(const ReturnStmt& stmt);

    void operator()(const IfStmt& stmt);

    void operator()(const BreakStmt& stmt);

    void operator()(const ContinueStmt& stmt);

    void operator()(const TryCatchContinueStmt& stmt);

    // Handles standalone expressions (e.g., `a + b;`)
    void operator()(const ExpressionStmt& exprStatement);

    // Handles variable declarations (`let x = 42;`)
    void operator()(const LetStmt& letStatement);

    // Handles block statments ({ 'Collection of statements' })
    void operator()(const BlockStmt& blockStatement);
    void operator()(const ClassStmt& classStatement);

    void operator()(const Binary& expr);
    void operator()(const Logical& expr);
    void operator()(const Conditional& expr);
    void operator()(const Unary& expr);
    void operator()(const Literal& expr);
    void operator()(const Grouping& expr);
    void operator()(const Variable& expr, ExprPtr exprPtr);
    void operator()(const Assignment& expr, ExprPtr exprPtr);
    void operator()(const Lambda& expr);
    void operator()(const Call& expr);
    void operator()(const Get& expr);
    void operator()(const Set& expr);

    void resolve(std::vector<std::shared_ptr<Statement>> statements);
    void resolve(std::shared_ptr<Statement> stmt);
    void resolve(ExprPtr expr);
    void resolveLocal(ExprPtr expr, Token name);
    void resolveFunction(const FunctionStmt &stmt, FunctionType type);

    void beginScope();
    void endScope();

    void declare(Token name);
    void define(Token name);

    Resolver(std::shared_ptr<Interpreter> interpreter) 
        : interpreter(interpreter), currentFunction(FunctionType::NONE) {}
};