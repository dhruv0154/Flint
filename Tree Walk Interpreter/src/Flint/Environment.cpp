#include "Flint/Environment.h"
#include "Flint/Exceptions/RuntimeError.h"
#include "Flint/Interpreter/Interpreter.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Environment::define
// ─────────────────────────────────────────────────────────────────────────────
//  Adds a new variable to the current environment/scope.
//  - @param name  : variable name as a string
//  - @param value : associated LiteralValue
//
//  This does not check for redefinition or shadowing—each environment maintains
//  its own local variables independently.
// ─────────────────────────────────────────────────────────────────────────────
void Environment::define(std::string name, LiteralValue value)
{
    values[name] = value;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Environment::get
// ─────────────────────────────────────────────────────────────────────────────
//  Looks up a variable by name, starting from this environment and walking
//  outward through enclosing environments if necessary.
//  - @param name : token containing variable name and source info (for errors)
//
//  Throws:
//      RuntimeError if variable doesn't exist or is uninitialized.
//
//  Handles:
//      - `std::monostate` (declared but uninitialized variables)
//      - `nullptr` explicitly assigned
// ─────────────────────────────────────────────────────────────────────────────
LiteralValue Environment::get(Token name)
{
    if (values.count(name.lexeme)) 
    {
        // Check if variable exists but is uninitialized
        if(std::holds_alternative<std::nullptr_t>(values[name.lexeme]) 
            || std::holds_alternative<std::monostate>(values[name.lexeme]))
        {
            throw RuntimeError(name, "Variable '" + name.lexeme + "' is undefined \033[33m(just like your future)\033[0m");
        }
        return values[name.lexeme];
    }
    // Recursively check enclosing scopes
    if(enclosing) return enclosing -> get(name);

    throw RuntimeError(name, "Unknown variable '" + name.lexeme + "'.");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Environment::getAt
// ─────────────────────────────────────────────────────────────────────────────
//  Efficient variable lookup when scope depth is known (via resolver).
//  - @param distance : number of environments to go outward
//  - @param name     : variable token (for key lookup)
//
//  Bypasses recursive search by jumping directly to ancestor environment.
// ─────────────────────────────────────────────────────────────────────────────
LiteralValue Environment::getAt(int distance, Token name)
{
    return ancestors(distance)->values[name.lexeme];
}

// ─────────────────────────────────────────────────────────────────────────────
//  Environment::ancestors
// ─────────────────────────────────────────────────────────────────────────────
//  Utility to find the ancestor environment `distance` steps above.
//  - @param distance : number of steps to climb enclosing scopes
//
//  Returns:
//      shared_ptr to the target ancestor environment
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Environment> Environment::ancestors(int distance)
{
    std::shared_ptr<Environment> environment = shared_from_this();
    for(int i = 0; i < distance; i++) 
        environment = environment->enclosing;
    return environment;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Environment::getOptional
// ─────────────────────────────────────────────────────────────────────────────
//  Safe lookup that returns std::optional instead of throwing errors.
//  - @param name : variable name (string)
//
//  Returns:
//      std::optional<LiteralValue> — nullopt if variable not found.
// ─────────────────────────────────────────────────────────────────────────────
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
//  Environment::assign
// ─────────────────────────────────────────────────────────────────────────────
//  Reassigns value to a variable, searching through enclosing scopes.
//  - @param name  : variable token (includes lexeme for key + error context)
//  - @param value : new LiteralValue
//
//  Throws:
//      RuntimeError if variable not found in any enclosing scope.
//
//  Supports mutation of outer scopes (closure support).
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
        enclosing->assign(name, value);
        return;
    }

    throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Environment::assignAt
// ─────────────────────────────────────────────────────────────────────────────
//  Efficient value assignment at known distance (used with resolver).
//  - @param distance : scope depth
//  - @param name     : variable token
//  - @param value    : value to set
//
//  Avoids recursive lookup.
// ─────────────────────────────────────────────────────────────────────────────
void Environment::assignAt(int distance, Token name, LiteralValue value)
{
    ancestors(distance)->values[name.lexeme] = value;
}