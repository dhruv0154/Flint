#include "Flint/Environment.h"
#include "RuntimeError.h"
#include "Interpreter/Interpreter.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Environment::define
// ─────────────────────────────────────────────────────────────────────────────
//  Adds a new variable in the current scope.
//  - @param name  : variable name as a string
//  - @param value : associated value (LiteralValue)
//
//  Usage:
//      env.define("x", 42);
// ─────────────────────────────────────────────────────────────────────────────
void Environment::define(std::string name, LiteralValue value)
{
    values[name] = value;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Environment::get
// ─────────────────────────────────────────────────────────────────────────────
//  Retrieves the value bound to a variable from the current environment.
//  - @param name : token representing the variable (used for error context)
//
//  Throws:
//      RuntimeError if the variable is not found.
//
//  Example:
//      LiteralValue val = env.get(token);
//
//  Error message includes:
//    - The offending token's line and lexeme
// ─────────────────────────────────────────────────────────────────────────────
LiteralValue Environment::get(Token name)
{
    if (values.count(name.lexeme)) 
    {
        if(std::holds_alternative<nullptr_t>(values[name.lexeme]) 
            || std::holds_alternative<std::monostate>(values[name.lexeme]))
        {
            throw RuntimeError(name, "Variable '" + name.lexeme + "' has no value assigned to it.");
        }
        return values[name.lexeme];
    }
    if(enclosing) return enclosing -> get(name);

    throw RuntimeError(name, "Unknown variable '" + name.lexeme + "'.");
}

LiteralValue Environment::getAt(int distance, Token name)
{
    return ancestors(distance) -> values[name.lexeme];
}

std::shared_ptr<Environment> Environment::ancestors(int distance)
{
    std::shared_ptr<Environment> environment = shared_from_this();
    for(int i = 0; i < distance; i++) environment = environment -> enclosing;
    return environment;
}

std::optional<LiteralValue> Environment::getOptional(const std::string& name) const 
{
    if (values.find(name) != values.end()) {
        return values.at(name);
    }
    if (enclosing) {
        return enclosing->getOptional(name);
    }
    return std::nullopt;
}


// ─────────────────────────────────────────────────────────────────────────────
//  Environment::define
// ─────────────────────────────────────────────────────────────────────────────
//  Reassign a new value to an exisitng variable in current scope.
//  - @param name  : variable name as a Token
//  - @param value : new value (LiteralValue)
//
//  Throws:
//      RuntimeError if the variable is not found.
//  Usage:
//      env.assign(token, 42); 
// ─────────────────────────────────────────────────────────────────────────────
void Environment::assign(Token name, LiteralValue value)
{
    if(values.count(name.lexeme))
    {
        values[name.lexeme] = value;
        return;
    }

    if(enclosing) 
    {
        enclosing -> assign(name, value);
        return;
    }

    throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
}

void Environment::assignAt(int distance, Token name, LiteralValue value)
{
    ancestors(distance) -> values[name.lexeme] = value;
}