#include "Resolver.h"
#include "Flint/Flint.h"

void Resolver::operator()(const BlockStmt &stmt)
{
    beginScope();
    resolve(stmt.statements);
    endScope();
}

void Resolver::operator()(const LetStmt &stmt)
{
    for (auto& [name, initializer] : stmt.declarations)
    {
        declare(name);
    }
    for (auto& [name, initializer] : stmt.declarations)
    {
        if (initializer)
            resolve(initializer);
        define(name);
    }
}

void Resolver::operator()(const FunctionStmt &stmt)
{
    if(stmt.name.has_value())
    {
        declare(stmt.name.value());
        define(stmt.name.value());
    }

    resolveFunction(stmt, FunctionType::FUNCTION);
}

void Resolver::operator()(const ExpressionStmt &stmt)
{
    resolve(stmt.expression);
}

void Resolver::operator()(const IfStmt &stmt)
{
    resolve(stmt.condition);
    resolve(stmt.thenBranch);
    if(stmt.elseBranch) resolve(stmt.elseBranch);
}

void Resolver::operator()(const TryCatchContinueStmt &stmt)
{
    resolve(stmt.body);
}

void Resolver::operator()(const ReturnStmt &stmt)
{
    if(currentFunction == FunctionType::NONE)
    {
        Flint::error(stmt.keyword, "Can't return from outside a function.");
    }
    if(currentFunction == FunctionType::INITIALIZER)
    {
        Flint::error(stmt.keyword, "Can't return from an initializer.");
    }
    if(stmt.val) resolve(stmt.val);
}

void Resolver::operator()(const WhileStmt &stmt)
{
    resolve(stmt.condition);
    resolve(stmt.statement);
}

void Resolver::operator()(const ContinueStmt &stmt)
{
    return;
}

void Resolver::operator()(const BreakStmt &stmt)
{
    return;
}

void Resolver::operator()(const Variable &expr, ExprPtr exprPtr)
{
    if(!scopes.empty())
    {
        std::unordered_map<std::string, bool> &scope = scopes.back();
        auto it = scope.find(expr.name.lexeme);
        if(it != scope.end() && !it -> second) Flint::error(expr.name, 
                "Cannot read local variable '" + 
                    expr.name.lexeme + "' in its own initializer.!");
    }

    resolveLocal(exprPtr, expr.name);
}

void Resolver::operator()(const ClassStmt& classStatement)
{
    ClassType enclosingClass = currentClass;
    currentClass = ClassType::CLASS;
    declare(classStatement.name);
    define(classStatement.name);

    beginScope();
    scopes.back()["this"] = true;

    for (auto &&i : classStatement.methods)
    {
        FunctionType declaration = FunctionType::METHOD;
        FunctionStmt method = std::get<FunctionStmt>(*i);
        if(method.name -> lexeme == "init") declaration = FunctionType::INITIALIZER;
        resolveFunction(method, declaration);
    }
    currentClass = enclosingClass;
    endScope();
}

void Resolver::operator()(const Binary &expr)
{
    resolve(expr.left);
    resolve(expr.right);
}

void Resolver::operator()(const Call &expr)
{
    resolve(expr.callee);

    for (ExprPtr argument : expr.arguments)
    {
        resolve(argument);
    }
}

void Resolver::operator()(const Grouping &expr)
{
    resolve(expr.expression);
}

void Resolver::operator()(const Assignment &expr, ExprPtr exprPtr)
{
    resolve(expr.value);
    resolveLocal(exprPtr, expr.name);
}

void Resolver::operator()(const Literal &expr)
{
    return;
}

void Resolver::operator()(const Logical &expr)
{
    resolve(expr.right);
    resolve(expr.left);
}

void Resolver::operator()(const Conditional &expr)
{
    resolve(expr.condition);
    resolve(expr.left);
    resolve(expr.right);
}

void Resolver::operator()(const Lambda &expr)
{
    resolveFunction(*expr.function.get(), FunctionType::LAMBDA);
}

void Resolver::operator()(const Unary &expr)
{
    resolve(expr.right);
}

void Resolver::operator()(const Get& expr)
{
    resolve(expr.object);
}

void Resolver::operator()(const Set& expr)
{
    resolve(expr.value);
    resolve(expr.object);
}

void Resolver::operator()(const This& expr, ExprPtr exprPtr)
{
    if(currentClass == ClassType::NONE) 
        Flint::error(expr.keyword, "Use of 'this' outside a class is not allowed.");
    resolveLocal(exprPtr, expr.keyword);
}

void Resolver::resolveLocal(ExprPtr expr, Token name)
{
    for (int i = scopes.size() - 1; i >= 0; --i)
    {
        if(scopes[i].count(name.lexeme))
        {
            interpreter -> resolve(expr, scopes.size() - 1 - i);
            return;
        }
    }
    
}

void Resolver::resolveFunction(const FunctionStmt &stmt, FunctionType type)
{ 
    FunctionType enclosingFunction = currentFunction;
    currentFunction = type;  
    beginScope();
    for (Token param : stmt.params)
    {
        declare(param);
        define(param);
    }
    resolve(stmt.body);
    endScope();
    currentFunction = enclosingFunction;
}

void Resolver::declare(Token name)
{
    if(scopes.empty()) return;

    std::unordered_map<std::string, bool> &scope = scopes.back();
    if(scope.count(name.lexeme)) 
        Flint::error(name, 
            "Variable '" + name.lexeme + "' is already declared in this scope" );

    scope[name.lexeme] = false;
}

void Resolver::define(Token name)
{
    if(scopes.empty()) return;

    scopes.back()[name.lexeme] = true;
}

void Resolver::beginScope()
{
    std::unordered_map<std::string, bool> map;
    scopes.push_back(map);
}

void Resolver::endScope()
{
    scopes.pop_back();
}

void Resolver::resolve(std::vector<std::shared_ptr<Statement>> statements)
{
    for (auto &&statement : statements)
    {
        resolve(statement);
    }
}

void Resolver::resolve(std::shared_ptr<Statement> statement)
{
    std::visit(*this, *statement.get());
}

void Resolver::resolve(ExprPtr expr) 
{
    std::visit([&](auto& actualExpr) {
        using T = std::decay_t<decltype(actualExpr)>;
        if constexpr (std::is_same_v<T, Variable> 
            || std::is_same_v<T, Assignment> || 
                std::is_same_v<T, This>)
            (*this)(actualExpr, expr);
        else
            (*this)(actualExpr);     
    }, *expr);
}