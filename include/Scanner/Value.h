#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  LiteralValue Type
// ─────────────────────────────────────────────────────────────────────────────
//  Represents any literal or runtime value in the Flint language.
//
//  This type is used for:
//   - Token literal values (e.g., numbers, strings)
//   - Variable storage in the Environment
//   - Evaluation results in the interpreter
//
//  Internally uses `std::variant`, a type-safe union to support multiple types.
//
//  Supported Types:
//   - std::monostate   → default uninitialized state (useful for error handling)
//   - double           → numeric literals with decimals
//   - int              → integer literals (if separately tokenized)
//   - std::string      → string values
//   - std::nullptr_t   → `nothing` keyword (null-equivalent)
//   - bool             → true / false
// ─────────────────────────────────────────────────────────────────────────────

#include <variant>
#include <string>
#include <memory>
#include <vector>
#include <iostream>

class FlintCallable;

using LiteralValue = std::variant<
    std::monostate,   // Represents "no value" (used when token has no literal)
    double,           // Floating-point numbers (e.g., 3.14)
    std::string,      // String literals (e.g., "hello")
    std::nullptr_t,   // Represents Flint's `nothing` (like `null` or `nil`)
    bool,              // Boolean values: true, false
    std::shared_ptr<FlintCallable>
>;