#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  Environment Class
//  ─────────────────────────────────────────────────────────────────────────────
//  This class represents a *runtime environment* for the Flint language.
//  It maps variable names (strings) to their corresponding runtime values,
//  which are stored as `LiteralValue` (a std::variant: double, bool, nil, etc.).
//
//  Used during interpretation of variable expressions and declarations.
//
//  ────────────────
//  Dependencies:
//  - Scanner/Value.h   → Defines LiteralValue (std::variant of primitive types).
//  - Scanner/Token.h   → Used for error reporting and variable name tracking.
// ─────────────────────────────────────────────────────────────────────────────

#include <unordered_map>
#include "Scanner/Value.h"
#include "Scanner/Token.h"

class Environment
{
private:
    // ─────────────────────────────────────────────────────────────────────
    // values:
    // The core storage for the environment.
    // Maps variable names (as strings) to their associated LiteralValue.
    // This mimics variable scoping like in dynamic languages.
    // ─────────────────────────────────────────────────────────────────────
    std::unordered_map<std::string, LiteralValue> values;

public:
    // ─────────────────────────────────────────────────────────────────────
    // define(name, value):
    // Adds a new variable or reassigns an existing one in the environment.
    // Used when evaluating `let` statements or assignments.
    //
    // Example:
    //     env.define("x", 42.0);  // let x = 42;
    // ─────────────────────────────────────────────────────────────────────
    void define(std::string name, LiteralValue value);

    // ─────────────────────────────────────────────────────────────────────
    // get(token):
    // Retrieves the value bound to the variable name represented by the token.
    // Throws a runtime error if the variable is undefined.
    //
    // Used during evaluation of variable expressions (`VariableExpr`).
    //
    // Example:
    //     Token name = ...;  // token for variable 'x'
    //     LiteralValue val = env.get(name);
    // ─────────────────────────────────────────────────────────────────────
    LiteralValue get(Token name);
};