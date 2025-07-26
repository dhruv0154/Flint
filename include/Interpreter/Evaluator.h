#pragma once  // Ensure single inclusion

// ─────────────────────────────────────────────────────────────────────────────
//  Evaluator.h – Expression Evaluation for Flint
// ─────────────────────────────────────────────────────────────────────────────
//  Defines the Evaluator class, which implements the visitor pattern
//  to compute the value of each AST expression node without side effects.
//  Used internally by the Interpreter to evaluate expressions.
//
//  Key responsibilities:
//    - Evaluate expressions (Binary, Unary, Literal, etc.)
//    - Enforce type checks and truthiness rules
//    - Perform variable lookups via environment chain
//    - Handle function calls, property access, and assignments
// ─────────────────────────────────────────────────────────────────────────────

#include <memory>
#include <cmath>
#include "ExpressionNode.h"           // AST node definitions (ExprPtr)
#include "Flint/Environment.h"        // For variable resolution

class Interpreter;  // Forward declare to avoid cyclic include

class Evaluator {
public:
    Interpreter& interpreter;  // Reference to the Interpreter invoking this evaluator

    //──────────────────────────────────────────────────────────────────────────
    // Expression Visitors (overloaded operator() for each AST node type)
    // Called via std::visit on ExpressionNode variant.
    //──────────────────────────────────────────────────────────────────────────

    // Compute a + b, a - b, comparisons, equality, etc.
    LiteralValue operator()(const Binary& expr) const;

    // Compute logical AND/OR with short-circuiting.
    LiteralValue operator()(const Logical& expr) const;

    // Compute ternary expressions: condition ? left : right.
    LiteralValue operator()(const Conditional& expr) const;

    // Compute unary operations: !value, -value.
    LiteralValue operator()(const Unary& expr) const;

    // Return the literal value (numbers, strings, booleans, nil).
    LiteralValue operator()(const Literal& expr) const;

    // Evaluate grouped sub-expression: (expr).
    LiteralValue operator()(const Grouping& expr) const;

    // Look up variable value in appropriate environment.
    LiteralValue operator()(const Variable& expr, ExprPtr exprPtr) const;

    // Evaluate assignment: update variable and return new value.
    LiteralValue operator()(const Assignment& expr, ExprPtr exprPtr) const;

    // Create a callable for a lambda expression.
    LiteralValue operator()(const Lambda& expr) const;

    // Invoke a function, class constructor, or getter.
    LiteralValue operator()(const Call& expr) const;

    // Access a property on an object: object.name.
    LiteralValue operator()(const Get& expr) const;

    // Assign to a property: object.name = value.
    LiteralValue operator()(const Set& expr) const;

    // Handle 'this' keyword to reference current instance.
    LiteralValue operator()(const This& expr, ExprPtr exprPtr) const;

    // Handle 'super' keyword to reference the super class.
    LiteralValue operator()(const Super& expr, ExprPtr exprPtr) const;

    //──────────────────────────────────────────────────────────────────────────
    // Evaluation Helpers
    //──────────────────────────────────────────────────────────────────────────

    // Entry point: evaluate an ExprPtr by dispatching to the above visitors.
    LiteralValue evaluate(const ExprPtr& expr) const;

    //──────────────────────────────────────────────────────────────────────────
    // Utility Functions
    //──────────────────────────────────────────────────────────────────────────

    // Determine truthiness: nil, false, and numeric zero are false; others true.
    bool isTruthy(const LiteralValue& value) const;

    // Resolve variable using static analysis distance or default lookup.
    LiteralValue lookUpVariable(Token name, ExprPtr expr) const;

    // Compare two values for equality (handles numeric and other types).
    bool isEqual(const LiteralValue& left, const LiteralValue& right) const;

    // Enforce that operands match expected types (e.g., numbers for +).
    template<typename... Operands>
    void checkOperandType(const Token& op, const Operands&... operands) const;

    //──────────────────────────────────────────────────────────────────────────
    // Constructor: binds this evaluator to its interpreter
    //──────────────────────────────────────────────────────────────────────────
    explicit Evaluator(Interpreter& interpreter)
        : interpreter(interpreter) {}
};