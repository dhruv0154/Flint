#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  Value.h – Runtime Value Representation for Flint
// ─────────────────────────────────────────────────────────────────────────────
//  Defines LiteralValue, a variant type that can hold any value produced
//  or manipulated by Flint programs at runtime.
//
//  Use cases:
//    - Token literal values (numbers, strings, booleans, nothing)
//    - Storage in variable environments
//    - Results returned by expression evaluation
// ─────────────────────────────────────────────────────────────────────────────

#include <variant>
#include <string>
#include <memory>
#include <vector>
#include <iostream>

// Forward declarations for callable/class/instance types
class FlintCallable;
class FlintClass;
class FlintInstance;
class FlintArray;


// LiteralValue variant holds all primitive and object types:
using LiteralValue = std::variant<
    std::monostate,                // Default uninitialized state, signals absence of value
    double,                        // Floating-point numeric literals (e.g., 3.14)
    std::string,                   // String literals (e.g., "hello world")
    std::nullptr_t,                // ‘nothing’ keyword (null/nil equivalent)
    bool,                          // Boolean literals: true or false
    std::shared_ptr<FlintCallable>,// Functions or native-callable objects
    std::shared_ptr<FlintClass>,   // Class definitions (for instantiation)
    std::shared_ptr<FlintInstance>, // Object instances with fields and methods
    std::shared_ptr<FlintArray>   // For arrays
>;