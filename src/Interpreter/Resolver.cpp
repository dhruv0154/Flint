#include "Resolver.h"
#include "Flint/Flint.h"

// ─────────────────────────────────────────────────────────────────────────────
// Resolver
// Performs a static, lexical‐scope analysis of AST nodes to:
//  • detect illegal uses (e.g., return outside function),
//  • resolve each variable and “this” access to its lexical depth,
//  • prepare the Interpreter’s locals map for fast lookups.
// ─────────────────────────────────────────────────────────────────────────────

// BlockStmt: each block opens a new scope, resolves its statements, then closes.
void Resolver::operator()(const BlockStmt &stmt)
{
    beginScope();              // push a fresh scope map
    resolve(stmt.statements);  // resolve inner statements
    endScope();                // pop back to outer scope
}

// LetStmt: two‐phase for declarations & initializers
void Resolver::operator()(const LetStmt &stmt)
{
    // Phase 1: declare all variables (marks them as 'declared but uninitialized')
    for (auto& [name, initializer] : stmt.declarations) {
        declare(name);
    }
    // Phase 2: resolve initializers, then mark as defined
    for (auto& [name, initializer] : stmt.declarations) {
        if (initializer)
            resolve(initializer);  // resolve RHS expression
        define(name);              // mark var as fully initialized
    }
}

// FunctionStmt: declare the function name, then resolve its body
void Resolver::operator()(const FunctionStmt &stmt)
{
    if (stmt.name.has_value()) {
        declare(stmt.name.value());  // makes the name visible in outer scope
        define(stmt.name.value());   // marks it ready for calls (allows recursion)
    }
    resolveFunction(stmt, FunctionType::FUNCTION);
}

// ExpressionStmt: just resolve the contained expression
void Resolver::operator()(const ExpressionStmt &stmt)
{
    resolve(stmt.expression);
}

// IfStmt: resolve condition, then branches
void Resolver::operator()(const IfStmt &stmt)
{
    resolve(stmt.condition);
    resolve(stmt.thenBranch);
    if (stmt.elseBranch)
        resolve(stmt.elseBranch);
}

// TryCatchContinueStmt: we only care about body (we’ll catch ContinueExceptions at runtime)
void Resolver::operator()(const TryCatchContinueStmt &stmt)
{
    resolve(stmt.body);
}

// ReturnStmt: ensure we're inside a function; initializers forbid return
void Resolver::operator()(const ReturnStmt &stmt)
{
    if (currentFunction == FunctionType::NONE)
        Flint::error(stmt.keyword, "Can't return from outside a function.");
    if (currentFunction == FunctionType::INITIALIZER)
        Flint::error(stmt.keyword, "Can't return from an initializer.");
    if (stmt.val)
        resolve(stmt.val);  // resolve return expression if any
}

// WhileStmt: resolve loop condition and body
void Resolver::operator()(const WhileStmt &stmt)
{
    resolve(stmt.condition);
    resolve(stmt.statement);
}

// ContinueStmt & BreakStmt: no resolution needed, runtime handled
void Resolver::operator()(const ContinueStmt &stmt) { /* nothing */ }
void Resolver::operator()(const BreakStmt &stmt)    { /* nothing */ }

// Variable expr: check for self‐reference in initializer, then record its depth
void Resolver::operator()(const Variable &expr, ExprPtr exprPtr)
{
    if (!scopes.empty()) {
        auto &scope = scopes.back();
        auto it = scope.find(expr.name.lexeme);
        // if declared but not yet defined → illegal read in its own init
        if (it != scope.end() && !it->second)
            Flint::error(expr.name,
                "Cannot read local variable '" + expr.name.lexeme +
                "' in its own initializer.");
    }
    resolveLocal(exprPtr, expr.name);
}

// ClassStmt: handles static & instance methods, sets up ‘this’ binding
void Resolver::operator()(const ClassStmt& classStatement)
{
    auto enclosing = currentClass;
    currentClass = ClassType::CLASS;

    declare(classStatement.name);  // placeholder so class name is in scope
    define(classStatement.name);   // now resolvable inside methods

    // ‘this’ is valid within instance‐method scopes
    beginScope();
    scopes.back()["this"] = true;

    // Resolve class (static) methods
    for (auto&& m : classStatement.classMethods) {
        FunctionStmt method = std::get<FunctionStmt>(*m);
        auto type = FunctionType::METHOD;
        if (method.name->lexeme == "init") type = FunctionType::INITIALIZER;
        resolveFunction(method, type);
    }
    // Resolve instance methods
    for (auto&& m : classStatement.instanceMethods) {
        FunctionStmt method = std::get<FunctionStmt>(*m);
        auto type = FunctionType::METHOD;
        if (method.name->lexeme == "init") type = FunctionType::INITIALIZER;
        resolveFunction(method, type);
    }

    endScope();
    currentClass = enclosing;
}

// Binary expr: resolve subexpressions
void Resolver::operator()(const Binary &expr)
{
    resolve(expr.left);
    resolve(expr.right);
}

// Call expr: resolve the callee and each argument
void Resolver::operator()(const Call &expr)
{
    resolve(expr.callee);
    for (auto& arg : expr.arguments)
        resolve(arg);
}

// Grouping expr: just resolve its inner expression
void Resolver::operator()(const Grouping &expr)
{
    resolve(expr.expression);
}

// Assignment expr: resolve RHS, then register the write at correct depth
void Resolver::operator()(const Assignment &expr, ExprPtr exprPtr)
{
    resolve(expr.value);
    resolveLocal(exprPtr, expr.name);
}

// Literal expr: nothing to do
void Resolver::operator()(const Literal &expr) { /* nothing */ }

// Logical expr: resolve both sides (short‐circuit handled at runtime)
void Resolver::operator()(const Logical &expr)
{
    resolve(expr.left);
    resolve(expr.right);
}

// Ternary conditional: resolve condition and both branches
void Resolver::operator()(const Conditional &expr)
{
    resolve(expr.condition);
    resolve(expr.left);
    resolve(expr.right);
}

// Lambda expr: treat as a function body with its own scope
void Resolver::operator()(const Lambda &expr)
{
    resolveFunction(*expr.function.get(), FunctionType::LAMBDA);
}

// Unary expr: resolve the single operand
void Resolver::operator()(const Unary &expr)
{
    resolve(expr.right);
}

// Get expr: resolve the object whose property is accessed
void Resolver::operator()(const Get& expr)
{
    resolve(expr.object);
}

// Set expr: resolve both the target object and the value being set
void Resolver::operator()(const Set& expr)
{
    resolve(expr.value);
    resolve(expr.object);
}

// This expr: ensure within a class, then record it
void Resolver::operator()(const This& expr, ExprPtr exprPtr)
{
    if (currentClass == ClassType::NONE)
        Flint::error(expr.keyword,
                     "Use of 'this' outside a class is not allowed.");
    resolveLocal(exprPtr, expr.keyword);
}

// resolveLocal: find the nearest scope containing the name and tell
// the interpreter the lexical distance for fast lookups at runtime.
void Resolver::resolveLocal(ExprPtr expr, Token name)
{
    for (int i = scopes.size() - 1; i >= 0; --i) {
        if (scopes[i].count(name.lexeme)) {
            interpreter->resolve(expr, (int)scopes.size() - 1 - i);
            return;
        }
    }
    // if not found, it's global
}

// resolveFunction: open a new scope for parameters + body, remember
// what kind of function (method, initializer, etc.), then close.
void Resolver::resolveFunction(const FunctionStmt &stmt, FunctionType type)
{
    if (stmt.isGetter && type != FunctionType::METHOD)
        Flint::error(*stmt.name,
            "Use of getter/setter outside a class.");

    auto enclosing = currentFunction;
    currentFunction = type;

    beginScope();
    for (auto& param : stmt.params) {
        declare(param);
        define(param);
    }
    resolve(stmt.body);
    endScope();

    currentFunction = enclosing;
}

// declare: add a name to the current scope as 'declared but not yet defined'
void Resolver::declare(Token name)
{
    if (scopes.empty()) return;
    auto &scope = scopes.back();
    if (scope.count(name.lexeme))
        Flint::error(name,
            "Variable '" + name.lexeme + "' already declared in this scope.");
    scope[name.lexeme] = false;
}

// define: mark a name in the current scope as fully initialized
void Resolver::define(Token name)
{
    if (scopes.empty()) return;
    scopes.back()[name.lexeme] = true;
}

// scope management
void Resolver::beginScope() { scopes.emplace_back(); }
void Resolver::endScope()   { scopes.pop_back(); }

// Convenience overloads to kick off resolution on lists or single nodes
void Resolver::resolve(std::vector<std::shared_ptr<Statement>> statements)
{
    for (auto& stmt : statements) resolve(stmt);
}

void Resolver::resolve(std::shared_ptr<Statement> statement)
{
    std::visit(*this, *statement.get());
}

void Resolver::resolve(ExprPtr expr)
{
    std::visit([&](auto& actualExpr) {
        using T = std::decay_t<decltype(actualExpr)>;
        if constexpr (std::is_same_v<T, Variable> ||
                      std::is_same_v<T, Assignment> ||
                      std::is_same_v<T, This>)
        {
            (*this)(actualExpr, expr);
        } else {
            (*this)(actualExpr);
        }
    }, *expr);
}