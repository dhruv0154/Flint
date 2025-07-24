#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  RuntimeError Class
// ─────────────────────────────────────────────────────────────────────────────
//  Represents an error that occurs during interpretation (runtime), such as:
//    - Division by zero
//    - Invalid operations on mismatched types
//    - Accessing undefined variables
//
//  Contains both:
//    - A descriptive error message (via std::runtime_error)
//    - The offending token (for accurate error reporting)
//
//  This class is thrown and later caught during interpretation, 
//  typically inside the Evaluator or Interpreter.
// ─────────────────────────────────────────────────────────────────────────────

#include <iostream>
#include "Scanner/Token.h"

class RuntimeError : public std::runtime_error
{
public:
    Token token; // Token that caused the error (used for line number, lexeme, etc.)

    // ─────────────────────────────────────────────────────────────
    // Constructor
    // @param token   : Token causing the error (e.g., operator, identifier)
    // @param message : Human-readable explanation of what went wrong
    // ─────────────────────────────────────────────────────────────
    RuntimeError(Token token, const std::string& message)
        : std::runtime_error(message), token(token) {}
};