#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  Evaluator Class
//  ─────────────────────────────────────────────────────────────────────────────
//  This class implements the logic to evaluate each type of expression node
//  in the Flint AST (Abstract Syntax Tree).
//
//  Implements the visitor pattern via overloaded `operator()` functions for
//  each expression type (Binary, Unary, Literal, etc.).
//
//  Each method returns a `LiteralValue`, which is a variant holding runtime
//  values such as double, bool, string, or nil.
//
//  This class is called by the Interpreter, but is focused purely on computing
//  expression results — it has no side effects like printing or variable assignment.
// ─────────────────────────────────────────────────────────────────────────────

#include <memory>
#include <cmath>
#include "ExpressionNode.h"
#include "Flint/Environment.h"

class Interpreter;

class Evaluator 
{

public:
    Interpreter &interpreter;
    // ─────────────────────────────────────────────────────────────
    // Expression Visitors
    // Each handles a specific node type from the AST.
    // Called via std::visit on ExpressionNode's std::variant.
    // ─────────────────────────────────────────────────────────────

    // Evaluates binary expressions (e.g., a + b, a > b, a == b)
    LiteralValue operator()(const Binary& expr) const;

    // Evaluates logical 'or' and 'and' expressions (e.g. a == 0 && b == 1)
    LiteralValue operator()(const Logical& expr) const;

    // Evaluates conditional (ternary) expressions (e.g., cond ? a : b)
    LiteralValue operator()(const Conditional& expr) const;

    // Evaluates unary expressions (e.g., -a, !b)
    LiteralValue operator()(const Unary& expr) const;

    // Evaluates literals (e.g., numbers, booleans, nil)
    LiteralValue operator()(const Literal& expr) const;

    // Evaluates grouped expressions like (a + b)
    LiteralValue operator()(const Grouping& expr) const;

    // Evaluates a variable reference, typically looked up from environment
    LiteralValue operator()(const Variable& expr, ExprPtr exprPtr) const;

    // Evaluates a assignment expression like a = 1 + 5, a = b etc.
    LiteralValue operator()(const Assignment& expr, ExprPtr exprPtr) const;

    LiteralValue operator()(const Lambda& expr) const;

    LiteralValue operator()(const Call& expr) const;
    LiteralValue operator()(const Get& expr) const;
    LiteralValue operator()(const Set& expr) const;

    // ─────────────────────────────────────────────────────────────
    // Evaluation Helpers
    // These provide convenient entry points to evaluate a full expression.
    // ─────────────────────────────────────────────────────────────

    // Evaluate a pointer to an ExpressionNode (shared_ptr for AST trees)
    LiteralValue evaluate(const ExprPtr& expr) const;

    // ─────────────────────────────────────────────────────────────
    // Utility Functions
    // ─────────────────────────────────────────────────────────────

    // Determines if a value is "truthy" according to Flint's rules.
    // nothing, false and 0 → false; everything else → true.
    bool isTruthy(const LiteralValue& value) const;

    LiteralValue lookUpVariable(Token name, ExprPtr expr) const;

    // Compares two LiteralValues for equality (used in == operator).
    bool isEqual(const LiteralValue& left, const LiteralValue& right) const;

    // Ensures all operands are of a valid type (e.g., numeric for arithmetic).
    // Used in binary/unary op validation.
    template<typename... Operands>
    void checkOperandType(const Token& op, const Operands&... operands) const;

    Evaluator(Interpreter& interpreter)
        : interpreter(interpreter) {}

};