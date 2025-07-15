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
#include <optional>
#include "Scanner/Value.h"
#include "Scanner/Token.h"

class Environment : public std::enable_shared_from_this<Environment>
{
private:
    // ─────────────────────────────────────────────────────────────────────
    // values:
    // The core storage for the environment.
    // Maps variable names (as strings) to their associated LiteralValue.
    // ─────────────────────────────────────────────────────────────────────
    std::unordered_map<std::string, LiteralValue> values;

    // ─────────────────────────────────────────────────────────────────────
    // enclosing:
    // Pointer to the environment enclosing this one
    // This is for supporting variable scoping.
    // ─────────────────────────────────────────────────────────────────────
    std::shared_ptr<Environment> enclosing;

public:
    // ─────────────────────────────────────────────────────────────────────
    // define(name, value):
    // Adds a new variable.
    // Used when evaluating `let` statements.
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
    LiteralValue getAt(int distance, Token name);

    std::optional<LiteralValue> getOptional(const std::string &name) const;

    std::shared_ptr<Environment> ancestors(int distance);

    // ─────────────────────────────────────────────────────────────────────
    // assign(name, value):
    // reassigns an existing variable in the environment.
    // Used when evaluating assignments.
    //
    // Example:
    //     env.assign("x", 42.0);  // x = 42;
    // ─────────────────────────────────────────────────────────────────────
    void assign(Token name, LiteralValue value);
    void assignAt(int distance, Token name, LiteralValue value);

    // Default Constructor for no enclosing environment (global scope env).
    Environment() : enclosing(nullptr) {}
    // Overloaded Constructor for an enclosing environment.
    Environment(std::shared_ptr<Environment> enclosing) : enclosing(std::move(enclosing)) {}
};