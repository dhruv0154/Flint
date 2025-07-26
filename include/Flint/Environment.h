#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  Environment.h – Runtime Environment for Flint Variables
// ─────────────────────────────────────────────────────────────────────────────
//  Manages variable scopes during interpretation.  Each Environment maps
//  names to values (LiteralValue) and optionally chains to an enclosing
//  environment to implement nested scopes (blocks, functions, classes).
//
//  Core responsibilities:
//    - Define new variables (`let`)
//    - Lookup variables by name (current or parent scope)
//    - Assign to existing variables
//
//  Dependencies:
//    - Scanner/Value.h → Defines LiteralValue (variant of double, bool, string, etc.)
//    - Scanner/Token.h → Used for error reporting (undefined variables)
// ─────────────────────────────────────────────────────────────────────────────

#include <unordered_map>
#include <optional>
#include "Scanner/Value.h"   // LiteralValue
#include "Scanner/Token.h"   // Token for name & errors

class Environment : public std::enable_shared_from_this<Environment> {
private:
    //──────────────────────────────────────────────────────────────────────────
    // values: maps variable names to their runtime values.
    //──────────────────────────────────────────────────────────────────────────
    std::unordered_map<std::string, LiteralValue> values;

   

public:
    //──────────────────────────────────────────────────────────────────────────
    // enclosing: parent scope (nullptr for global scope).
    // Enables nested scope lookup: if not found locally, delegate.
    //──────────────────────────────────────────────────────────────────────────
    std::shared_ptr<Environment> enclosing;
    
    //──────────────────────────────────────────────────────────────────────────
    // Constructor: global scope (no enclosing)
    //──────────────────────────────────────────────────────────────────────────
    Environment() : enclosing(nullptr) {}

    //──────────────────────────────────────────────────────────────────────────
    // Constructor: nested scope with reference to parent environment.
    //──────────────────────────────────────────────────────────────────────────
    explicit Environment(std::shared_ptr<Environment> enclosing)
        : enclosing(std::move(enclosing)) {}

    //──────────────────────────────────────────────────────────────────────────
    // define: declare a new variable in this scope.
    // Usage: env.define("x", LiteralValue(42.0)); // let x = 42;
    //──────────────────────────────────────────────────────────────────────────
    void define(std::string name, LiteralValue value);

    //──────────────────────────────────────────────────────────────────────────
    // get: retrieve a variable's value, searching outward through enclosing.
    // Throws RuntimeError if name not found.
    //──────────────────────────────────────────────────────────────────────────
    LiteralValue get(Token name);

    //──────────────────────────────────────────────────────────────────────────
    // getAt: direct lookup in an ancestor environment at fixed distance.
    // Used by Resolver to handle statically-scoped variables.
    //──────────────────────────────────────────────────────────────────────────
    LiteralValue getAt(int distance, Token name);

    //──────────────────────────────────────────────────────────────────────────
    // getOptional: attempt to retrieve value in this scope only.
    // Returns std::nullopt if not present (does not search enclosing).
    //──────────────────────────────────────────────────────────────────────────
    std::optional<LiteralValue> getOptional(const std::string &name) const;

    //──────────────────────────────────────────────────────────────────────────
    // ancestors: return the environment `distance` levels up.
    //──────────────────────────────────────────────────────────────────────────
    std::shared_ptr<Environment> ancestors(int distance);

    //──────────────────────────────────────────────────────────────────────────
    // assign: rebind an existing variable (must exist in current or parent).
    // Throws RuntimeError if variable undefined.
    //──────────────────────────────────────────────────────────────────────────
    void assign(Token name, LiteralValue value);

    //──────────────────────────────────────────────────────────────────────────
    // assignAt: direct assignment in ancestor environment at given distance.
    // Used by Resolver for efficiency in statically-scoped resolution.
    //──────────────────────────────────────────────────────────────────────────
    void assignAt(int distance, Token name, LiteralValue value);
};