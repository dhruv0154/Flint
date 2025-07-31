#pragma once

#include <functional>
#include <string>
#include "Flint/Callables/FlintCallable.h"

// Represents a built-in (native) function callable from Flint code.
class NativeFunction : public FlintCallable 
{
public:
    // Type alias for the C++ function that implements the native behavior.
    // It accepts arguments and a token for error reporting (like a closing parenthesis).
    using NativeFn = std::function<LiteralValue
        (const std::vector<LiteralValue>&, 
         const Token &paren)>;

    // Constructor for a native function. Takes the function arity (number of parameters),
    // the actual native function implementation, and an optional name for debugging.
    NativeFunction(int arity, NativeFn fn, std::string name = "") 
        : arity_(arity), fn_(std::move(fn)), 
          name_(std::move(name)) {}

    // Returns the number of arguments the function expects.
    int arity() const override {
        return arity_;
    }

    // Calls the native function using the provided arguments.
    LiteralValue call(Interpreter& interpreter, 
        const std::vector<LiteralValue>& args, const Token &paren) override {
        return fn_(args, paren);
    }

    // Returns a string representation of the native function.
    std::string toString() const override {
        return "<native fn: " + name_ + ">";
    }

private:
    int arity_;         // Expected number of arguments.
    NativeFn fn_;       // The actual C++ function to be executed.
    std::string name_;  // Optional name (used in debugging).
};