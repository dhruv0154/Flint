#include "Interpreter/Evaluator.h"
#include "Stmt.h"
#include "Interpreter/Interpreter.h"
#include "RuntimeError.h"
#include "FlintCallable.h"
#include "FlintFunction.h"
#include "FlintInstance.h"

// ─────────────────────────────────────────────────────────────────────────────
// Binary Expression Evaluation
// Handles expressions like `a + b`, `x > y`, etc.
// ─────────────────────────────────────────────────────────────────────────────
LiteralValue Evaluator::operator()(const Binary& expr) const 
{
    LiteralValue left = evaluate(expr.left);
    LiteralValue right = evaluate(expr.right);
    std::string message;

    switch (expr.op.type) 
    {
        // Comma expression: evaluate both but return the right value
        case TokenType::COMMA:
            return right;
            
        // Arithmetic + string concatenation
        case TokenType::PLUS:
            if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
                return std::get<double>(left) + std::get<double>(right);
            // If either is a string, convert both to strings and concatenate
            else if (std::holds_alternative<std::string>(left) || std::holds_alternative<std::string>(right))
                return Interpreter::stringify(left) + Interpreter::stringify(right);

            message = "Operands to '+' must be both numbers or at least one string.";;
            throw RuntimeError(expr.op, message); 

        // Arithmetic operations
        case TokenType::MINUS:
            checkOperandType(expr.op, left, right);
            return std::get<double>(left) - std::get<double>(right);

        case TokenType::STAR:
            checkOperandType(expr.op, left, right);
            return std::get<double>(left) * std::get<double>(right);

        case TokenType::SLASH:
            checkOperandType(expr.op, left, right);
            if (std::get<double>(right) == 0) {
                message = "Division by zero is undefined.";
                throw RuntimeError(expr.op, message);
            }
            return std::get<double>(left) / std::get<double>(right);

        case TokenType::MODULO:
            checkOperandType(expr.op, left, right);
            if (std::get<double>(right) == 0) {
                message = "Division by zero is undefined.";
                throw RuntimeError(expr.op, message);
            }
            return std::fmod(std::get<double>(left), std::get<double>(right));

        // Comparison operators
        case TokenType::GREATER:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
                return std::get<double>(left) > std::get<double>(right);
            if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
                return std::get<std::string>(left) > std::get<std::string>(right);
            throw RuntimeError(expr.op, message);

        case TokenType::GREATER_EQUAL:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
                return std::get<double>(left) >= std::get<double>(right);
            if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
                return std::get<std::string>(left) >= std::get<std::string>(right);
            throw RuntimeError(expr.op, message);

        case TokenType::LESS:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
                return std::get<double>(left) < std::get<double>(right);
            if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
                return std::get<std::string>(left) < std::get<std::string>(right);
            throw RuntimeError(expr.op, message);

        case TokenType::LESS_EQUAL:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
                return std::get<double>(left) <= std::get<double>(right);
            if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
                return std::get<std::string>(left) <= std::get<std::string>(right);
            throw RuntimeError(expr.op, message);

        // Equality
        case TokenType::BANG_EQUAL:
            return !isEqual(left, right);
        case TokenType::EQUAL_EQUAL:
            return isEqual(left, right);

        default:
            break;
    }

    return std::monostate{}; // Fallback for unsupported operators
}

LiteralValue Evaluator::operator()(const Logical& expr) const
{
    LiteralValue left = evaluate(expr.left);
    
    if(expr.op.type == TokenType::OR)
    {
        if(isTruthy(left)) return left;
    }
    else
    {
        if(!isTruthy(left)) return left;
    }

    return evaluate(expr.right);
}

// ─────────────────────────────────────────────────────────────────────────────
// Ternary Conditional Evaluation: `condition ? trueExpr : falseExpr`
// ─────────────────────────────────────────────────────────────────────────────
LiteralValue Evaluator::operator()(const Conditional& expr) const 
{
    LiteralValue condition = evaluate(expr.condition);
    return isTruthy(condition) ? evaluate(expr.left) : evaluate(expr.right);
}

// ─────────────────────────────────────────────────────────────────────────────
// Unary Operator Evaluation: -value or !value
// ─────────────────────────────────────────────────────────────────────────────
LiteralValue Evaluator::operator()(const Unary& expr) const 
{
    LiteralValue right = evaluate(expr.right);

    if (expr.op.type == TokenType::MINUS)
    {
        checkOperandType(expr.op, right);
        return -std::get<double>(right);
    }

    if (expr.op.type == TokenType::BANG)
        return !isTruthy(right);

    return std::monostate{};
}

// ─────────────────────────────────────────────────────────────────────────────
// Literal Value (constants like numbers, strings, booleans)
// ─────────────────────────────────────────────────────────────────────────────
LiteralValue Evaluator::operator()(const Literal& expr) const 
{
    return expr.value;
}

// ─────────────────────────────────────────────────────────────────────────────
// Grouping (i.e., parentheses): `(a + b)` just returns the inner value
// ─────────────────────────────────────────────────────────────────────────────
LiteralValue Evaluator::operator()(const Grouping& expr) const 
{
    return evaluate(expr.expression);
}

// ─────────────────────────────────────────────────────────────────────────────
// Variable Lookup via current Interpreter's environment
// ─────────────────────────────────────────────────────────────────────────────
LiteralValue Evaluator::operator()(const Variable& expr, ExprPtr exprPtr) const
{
    return lookUpVariable(expr.name, exprPtr);
}

LiteralValue Evaluator::operator()(const Assignment& expr, ExprPtr exprPtr) const
{
    LiteralValue val = evaluate(expr.value);
    auto it = interpreter.locals.find(exprPtr);

    if(it != interpreter.locals.end())
    {
        int distance = it -> second;
        interpreter.environment -> assignAt(distance, expr.name, val);
    }
    else
    {
        interpreter.globals -> assign(expr.name, val);
    }

    return val;
}

LiteralValue Evaluator::operator()(const Lambda& expr) const
{
    std::shared_ptr<FlintFunction> fn = std::make_shared<FlintFunction>
        (expr.function, interpreter.environment, false);
    return fn;
}

LiteralValue Evaluator::operator()(const Call& expr) const
{
    LiteralValue callee = evaluate(expr.callee);

    std::vector<LiteralValue> arguments;
    for (ExprPtr argument : expr.arguments)
    {
        arguments.emplace_back(evaluate(argument));
    }

    if(!std::holds_alternative<std::shared_ptr<FlintCallable>>(callee))
        throw RuntimeError(expr.paren, 
            "Call to other types except classes and functions is not valid!");
    
    std::shared_ptr<FlintCallable> function = std::get<
        std::shared_ptr<FlintCallable>>(callee);

    if(function -> arity() != -1 && arguments.size() != function -> arity()) 
    {
        throw RuntimeError(expr.paren, 
        "Function expects " + std::to_string(function -> arity()) + 
        " arguments but got " + std::to_string(arguments.size()));
    }
    
    return function -> call(interpreter, arguments, expr.paren);
}

LiteralValue Evaluator::operator()(const Get& expr) const
{
    LiteralValue val = evaluate(expr.object);
    if(std::holds_alternative<std::shared_ptr<FlintInstance>>(val))
    {
        return std::get<std::shared_ptr<FlintInstance>>(val) -> get(expr.name);
    }

    throw RuntimeError(expr.name, "Only instances have properties.");
}

LiteralValue Evaluator::operator()(const Set& expr) const
{
    LiteralValue object = evaluate(expr.object);
    if(!std::holds_alternative<std::shared_ptr<FlintInstance>>(object))
    {
        throw RuntimeError(expr.name, "Only instances have fields.");
    }

    LiteralValue value = evaluate(expr.value);
    std::get<std::shared_ptr<FlintInstance>>(object) -> set(expr.name, value);
    return value;
}

LiteralValue Evaluator::operator()(const This& expr, ExprPtr exprPtr) const
{
    return lookUpVariable(expr.keyword, exprPtr);
}

LiteralValue Evaluator::evaluate(const ExprPtr& expr) const 
{
    if (!expr) return std::monostate{};

    LiteralValue result;
    std::visit([&](auto& actualExpr) {
        using T = std::decay_t<decltype(actualExpr)>;
        if constexpr (std::is_same_v<T, Variable> || 
            std::is_same_v<T, Assignment> || std::is_same_v<T, This>)
            result = (*this)(actualExpr, expr);  // Pass ExprPtr
        else
            result = (*this)(actualExpr);        // Regular
    }, *expr);

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Truthiness Evaluation
// Implements truth rules:
//   • `false`, `null`, and `nothing` → false
//   • everything else → true
// ─────────────────────────────────────────────────────────────────────────────
bool Evaluator::isTruthy(const LiteralValue& value) const 
{
    auto visitor = [] (auto&& val) -> bool
    {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, std::monostate>)      return false;
        else if constexpr (std::is_same_v<T, std::nullptr_t>) return false;
        else if constexpr (std::is_same_v<T, bool>)           return val;
        else if constexpr (std::is_same_v<T, double>)         return val != 0.0;
        else if constexpr (std::is_same_v<T, int>)            return val != 0;
        else if constexpr (std::is_same_v<T, std::string>)    return !val.empty();
        else if constexpr (std::is_same_v<T, std::shared_ptr<FlintCallable>>) return static_cast<bool>(val);
        else return true; // Fallback
    };

    return std::visit(visitor, value);
}

// ─────────────────────────────────────────────────────────────────────────────
// Equality Check between LiteralValues
// Used by == and != comparisons
// ─────────────────────────────────────────────────────────────────────────────
bool Evaluator::isEqual(const LiteralValue& left, const LiteralValue& right) const 
{
    if (std::holds_alternative<std::monostate>(left) && std::holds_alternative<std::monostate>(right))
        return true;
    if (std::holds_alternative<std::nullptr_t>(left) && std::holds_alternative<std::nullptr_t>(right))
        return true;
    if (std::holds_alternative<std::monostate>(left) || std::holds_alternative<std::nullptr_t>(left))
        return false;

    return left == right;
}

LiteralValue Evaluator::lookUpVariable(Token name, ExprPtr expr) const 
{
    auto it = interpreter.locals.find(expr);
    if (it != interpreter.locals.end()) {
        int distance = it->second;
        auto val = interpreter.environment->getAt(distance, name);
        return val;
    }
    auto val = interpreter.globals->get(name);
    return val;
}

// ─────────────────────────────────────────────────────────────────────────────
// Type Checker Utility for Arithmetic Ops
// Throws runtime error if operands aren’t numeric
// ─────────────────────────────────────────────────────────────────────────────
template<typename... Operands>
void Evaluator::checkOperandType(const Token& op, const Operands&... operands) const
{
    if ((... && std::holds_alternative<double>(operands))) return;

    std::string message = "Invalid operand type, operands must be numbers.";
    throw RuntimeError(op, message);
}