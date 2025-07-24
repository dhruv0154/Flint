// Interpreter/Evaluator.cpp
// Implements evaluation of expressions in the Flint interpreter.

#include "Interpreter/Evaluator.h"
#include "Stmt.h"
#include "Interpreter/Interpreter.h"
#include "RuntimeError.h"
#include "FlintCallable.h"
#include "FlintFunction.h"
#include "FlintInstance.h"
#include "FlintClass.h"

// ─────────────────────────────────────────────────────────────────────────────
// Binary Expression Evaluation
// Handles expressions like `a + b`, `x > y`, etc.
// ─────────────────────────────────────────────────────────────────────────────
LiteralValue Evaluator::operator()(const Binary& expr) const 
{
    // Evaluate left and right operands first
    LiteralValue left = evaluate(expr.left);
    LiteralValue right = evaluate(expr.right);
    std::string message;

    switch (expr.op.type) 
    {
        // Comma operator: discard left result, return right
        case TokenType::COMMA:
            return right;
            
        // '+' can mean numeric addition or string concatenation
        case TokenType::PLUS:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
                return std::get<double>(left) + std::get<double>(right);
            // If either operand is a string, coerce both to string and concatenate
            else if (std::holds_alternative<std::string>(left) || std::holds_alternative<std::string>(right))
                return Interpreter::stringify(left) + Interpreter::stringify(right);

            // Neither numeric nor string: runtime error
            message = "Operands to '+' must be both numbers or at least one string.";
            throw RuntimeError(expr.op, message);

        // Numeric subtraction: ensure both operands are numbers
        case TokenType::MINUS:
            checkOperandType(expr.op, left, right);
            return std::get<double>(left) - std::get<double>(right);

        // Numeric multiplication
        case TokenType::STAR:
            checkOperandType(expr.op, left, right);
            return std::get<double>(left) * std::get<double>(right);

        // Division: check for zero divisor
        case TokenType::SLASH:
            checkOperandType(expr.op, left, right);
            if (std::get<double>(right) == 0) {
                message = "Division by zero is undefined.";
                throw RuntimeError(expr.op, message);
            }
            return std::get<double>(left) / std::get<double>(right);

        // Modulo: uses std::fmod for floating-point operands
        case TokenType::MODULO:
            checkOperandType(expr.op, left, right);
            if (std::get<double>(right) == 0) {
                message = "Division by zero is undefined.";
                throw RuntimeError(expr.op, message);
            }
            return std::fmod(std::get<double>(left), std::get<double>(right));

        // Comparison: >, >=, <, <= operate on numbers or strings
        case TokenType::GREATER:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
                return std::get<double>(left) > std::get<double>(right);
            if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
                return std::get<std::string>(left) > std::get<std::string>(right);
            // Unsupported types: fall through to error
            message = "Operands to '>' must be both numbers or both strings.";
            throw RuntimeError(expr.op, message);

        case TokenType::GREATER_EQUAL:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
                return std::get<double>(left) >= std::get<double>(right);
            if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
                return std::get<std::string>(left) >= std::get<std::string>(right);
            message = "Operands to '>=' must be both numbers or both strings.";
            throw RuntimeError(expr.op, message);

        case TokenType::LESS:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
                return std::get<double>(left) < std::get<double>(right);
            if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
                return std::get<std::string>(left) < std::get<std::string>(right);
            message = "Operands to '<' must be both numbers or both strings.";
            throw RuntimeError(expr.op, message);

        case TokenType::LESS_EQUAL:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
                return std::get<double>(left) <= std::get<double>(right);
            if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
                return std::get<std::string>(left) <= std::get<std::string>(right);
            message = "Operands to '<=' must be both numbers or both strings.";
            throw RuntimeError(expr.op, message);

        // Equality and inequality: uses helper isEqual()
        case TokenType::BANG_EQUAL:
            return !isEqual(left, right);
        case TokenType::EQUAL_EQUAL:
            return isEqual(left, right);

        default:
            // Unhandled operator: return empty (should not reach here)
            break;
    }

    return std::monostate{}; // Fallback if no case matched
}

// Logical expressions: 'and' / 'or' with short-circuiting
LiteralValue Evaluator::operator()(const Logical& expr) const
{
    LiteralValue left = evaluate(expr.left);
    
    // For OR: if left is truthy, return it immediately
    if (expr.op.type == TokenType::OR)
    {
        if (isTruthy(left)) return left;
    }
    else // AND
    {
        // For AND: if left is falsy, return it immediately
        if (!isTruthy(left)) return left;
    }

    // Otherwise, evaluate and return right operand
    return evaluate(expr.right);
}

// ─────────────────────────────────────────────────────────────────────────────
// Ternary Conditional Evaluation: `condition ? trueExpr : falseExpr`
// ─────────────────────────────────────────────────────────────────────────────
LiteralValue Evaluator::operator()(const Conditional& expr) const 
{
    LiteralValue condition = evaluate(expr.condition);
    // Choose branch based on truthiness
    return isTruthy(condition)
        ? evaluate(expr.left)   // true branch
        : evaluate(expr.right); // false branch
}

// ─────────────────────────────────────────────────────────────────────────────
// Unary Operator Evaluation: -value or !value
// ─────────────────────────────────────────────────────────────────────────────
LiteralValue Evaluator::operator()(const Unary& expr) const 
{
    LiteralValue right = evaluate(expr.right);

    if (expr.op.type == TokenType::MINUS)
    {
        // Negation: ensure operand is numeric
        checkOperandType(expr.op, right);
        return -std::get<double>(right);
    }

    if (expr.op.type == TokenType::BANG)
        // Logical not: invert truthiness
        return !isTruthy(right);

    return std::monostate{};
}

// Literal constants: simply return the stored value
LiteralValue Evaluator::operator()(const Literal& expr) const 
{
    return expr.value;
}

// Grouping: parentheses, just evaluate inner expression
LiteralValue Evaluator::operator()(const Grouping& expr) const 
{
    return evaluate(expr.expression);
}

// Variable lookup: consult locals then globals
LiteralValue Evaluator::operator()(const Variable& expr, ExprPtr exprPtr) const
{
    return lookUpVariable(expr.name, exprPtr);
}

// Assignment: evaluate RHS, then store in appropriate environment
LiteralValue Evaluator::operator()(const Assignment& expr, ExprPtr exprPtr) const
{
    LiteralValue val = evaluate(expr.value);
    auto it = interpreter.locals.find(exprPtr);

    if (it != interpreter.locals.end())
    {
        // Local variable: assign at resolved depth
        int distance = it->second;
        interpreter.environment->assignAt(distance, expr.name, val);
    }
    else
    {
        // Global variable
        interpreter.globals->assign(expr.name, val);
    }

    return val;
}

// Lambda: wrap a function declaration into a callable object
LiteralValue Evaluator::operator()(const Lambda& expr) const
{
    auto fn = std::make_shared<FlintFunction>(
        expr.function,
        interpreter.environment,
        false // not a class initializer
    );
    return fn;
}

// Function or class call: evaluate callee and arguments, then invoke
LiteralValue Evaluator::operator()(const Call& expr) const
{
    LiteralValue callee = evaluate(expr.callee);
   
    // Evaluate all arguments before calling
    std::vector<LiteralValue> arguments;
    for (ExprPtr argument : expr.arguments)
    {
        arguments.push_back(evaluate(argument));
    }

    // Ensure callee is callable
    if (!std::holds_alternative<std::shared_ptr<FlintCallable>>(callee))
        throw RuntimeError(expr.paren,
            "Call to non-callable type is not valid!");
    
    auto function = std::get<std::shared_ptr<FlintCallable>>(callee);

    // Arity check: -1 for variadic
    if (function->arity() != -1 && arguments.size() != function->arity())
    {
        throw RuntimeError(expr.paren,
            "Function expects " + std::to_string(function->arity()) +
            " arguments but got " + std::to_string(arguments.size()));
    }

    // Invoke and return result
    return function->call(interpreter, arguments, expr.paren);
}

// Property access: distinguishing between classes and instances
LiteralValue Evaluator::operator()(const Get& expr) const
{
    LiteralValue val = evaluate(expr.object);
    // Accessing a static method on a class
    if (std::holds_alternative<std::shared_ptr<FlintCallable>>(val))
    {
        auto classPtr = std::dynamic_pointer_cast<FlintClass>(
            std::get<std::shared_ptr<FlintCallable>>(val)
        );
        if (classPtr)
            return classPtr->get(expr.name, interpreter);
    }
    // Accessing a field or method on an instance
    else if (std::holds_alternative<std::shared_ptr<FlintInstance>>(val))
    {
        return std::get<std::shared_ptr<FlintInstance>>(val)
            ->get(expr.name, interpreter);
    }

    throw RuntimeError(expr.name, "Only instances have properties.");
}

// Field assignment: evaluate target object and value, then store
LiteralValue Evaluator::operator()(const Set& expr) const
{
    LiteralValue object = evaluate(expr.object);
    if (!std::holds_alternative<std::shared_ptr<FlintInstance>>(object))
        throw RuntimeError(expr.name, "Only instances have fields.");

    LiteralValue value = evaluate(expr.value);
    std::get<std::shared_ptr<FlintInstance>>(object)
        ->set(expr.name, value);
    return value;
}

// 'this' reference resolution: behaves like variable lookup
LiteralValue Evaluator::operator()(const This& expr, ExprPtr exprPtr) const
{
    return lookUpVariable(expr.keyword, exprPtr);
}

// Core evaluate dispatcher: visits correct overload based on variant type
LiteralValue Evaluator::evaluate(const ExprPtr& expr) const 
{
    if (!expr) return std::monostate{};

    LiteralValue result;
    std::visit([&](auto& actualExpr) {
        using T = std::decay_t<decltype(actualExpr)>;
        if constexpr (std::is_same_v<T, Variable> ||
                      std::is_same_v<T, Assignment> ||
                      std::is_same_v<T, This>)
        {
            // Pass ExprPtr for these types
            result = (*this)(actualExpr, expr);
        }
        else
        {
            result = (*this)(actualExpr);
        }
    }, *expr);

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Truthiness Evaluation
// Implements truth rules:
//   • false, null/nothing -> false
//   • zero (numeric) -> false
//   • empty string -> false
//   • everything else -> true
// ─────────────────────────────────────────────────────────────────────────────
bool Evaluator::isTruthy(const LiteralValue& value) const 
{
    auto visitor = [](auto&& val) -> bool
    {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, std::monostate>)      return false;
        else if constexpr (std::is_same_v<T, std::nullptr_t>) return false;
        else if constexpr (std::is_same_v<T, bool>)           return val;
        else if constexpr (std::is_same_v<T, double>)         return val != 0.0;
        else if constexpr (std::is_same_v<T, int>)            return val != 0;
        else if constexpr (std::is_same_v<T, std::string>)    return !val.empty();
        else if constexpr (std::is_same_v<T, std::shared_ptr<FlintCallable>>)
            return static_cast<bool>(val);
        else return true; // objects, functions, etc.
    };

    return std::visit(visitor, value);
}

// ─────────────────────────────────────────────────────────────────────────────
// Equality Check between LiteralValues
// Ensures consistent semantics for == and !=
// ─────────────────────────────────────────────────────────────────────────────
bool Evaluator::isEqual(const LiteralValue& left, const LiteralValue& right) const 
{
    // both nullish or none -> equal
    if (std::holds_alternative<std::monostate>(left) && std::holds_alternative<std::monostate>(right))
        return true;
    if (std::holds_alternative<std::nullptr_t>(left) && std::holds_alternative<std::nullptr_t>(right))
        return true;
    // one nullish and one not -> not equal
    if (std::holds_alternative<std::monostate>(left) || std::holds_alternative<std::nullptr_t>(left))
        return false;

    // Direct variant comparison
    return left == right;
}

// Variable resolution helper: looks up locals first, then globals
LiteralValue Evaluator::lookUpVariable(Token name, ExprPtr expr) const 
{
    auto it = interpreter.locals.find(expr);
    if (it != interpreter.locals.end()) 
    {
        int distance = it->second;
        return interpreter.environment->getAt(distance, name);
    }
    // Fallback to global scope
    return interpreter.globals->get(name);
}

// ─────────────────────────────────────────────────────────────────────────────
// Type Checker Utility for Arithmetic Operations
// Throws if any operand is not a number (double)
// ─────────────────────────────────────────────────────────────────────────────
template<typename... Operands>
void Evaluator::checkOperandType(const Token& op, const Operands&... operands) const
{
    if ((... && std::holds_alternative<double>(operands))) return;

    std::string message = "Invalid operand type, operands must be numbers.";
    throw RuntimeError(op, message);
}