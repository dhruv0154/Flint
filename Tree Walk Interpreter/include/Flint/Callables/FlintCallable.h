#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  FlintCallable.h – Interface for Callable Entities in Flint
// ─────────────────────────────────────────────────────────────────────────────
//  Defines the abstract base class FlintCallable, representing any object 
//  that can be "called" like a function (e.g., user-defined functions, 
//  native functions, class constructors).
//
//  Any class implementing this interface must define:
//    - arity(): number of arguments it expects
//    - call(): the logic to invoke the function or callable
//    - toString(): optional string representation (defaults to "<fn>")
//
//  Used by the Interpreter to invoke functions uniformly, whether they're
//  written in Flint or implemented in C++.
// ─────────────────────────────────────────────────────────────────────────────

#include <vector>
#include <string>
#include "Flint/Scanner/Token.h"   // Required for passing the closing paren for error reporting
#include "Flint/Parser/Value.h"   // Defines LiteralValue (variant for function arguments)

class Interpreter;           // Forward declaration to avoid circular dependency

class FlintCallable {
public:
    //──────────────────────────────────────────────────────────────────────────
    // arity: returns number of arguments expected by the callable
    //──────────────────────────────────────────────────────────────────────────
    virtual int arity() const = 0;

    //──────────────────────────────────────────────────────────────────────────
    // call: executes the function/callable behavior
    //
    // Parameters:
    //   - interpreter: reference to the current interpreter instance
    //   - arguments: list of evaluated arguments to pass
    //   - paren: the closing parenthesis token (for runtime error context)
    //
    // Returns:
    //   - LiteralValue: result of the function call
    //──────────────────────────────────────────────────────────────────────────
    virtual LiteralValue call(Interpreter &interpreter, 
                              const std::vector<LiteralValue> &arguments,
                              const Token &paren) = 0;

    //──────────────────────────────────────────────────────────────────────────
    // toString: human-readable name for debugging (default: "<fn>")
    //──────────────────────────────────────────────────────────────────────────
    virtual std::string toString() const { return "<fn>"; }

    //──────────────────────────────────────────────────────────────────────────
    // Virtual destructor for safe polymorphic destruction
    //──────────────────────────────────────────────────────────────────────────
    virtual ~FlintCallable() = default;
};