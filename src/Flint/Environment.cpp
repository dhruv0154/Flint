#include "Flint/Environment.h"
#include "RuntimeError.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Environment::define
// ─────────────────────────────────────────────────────────────────────────────
//  Adds a new variable (or updates an existing one) in the current scope.
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
        return values[name.lexeme];

    throw RuntimeError(name, "Unknown variable '" + name.lexeme + "'.");
}