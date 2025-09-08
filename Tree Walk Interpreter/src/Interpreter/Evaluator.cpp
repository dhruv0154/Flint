#include <algorithm>
#include "Flint/Interpreter/Evaluator.h"
#include "Flint/ASTNodes/Stmt.h"
#include "Flint/Interpreter/Interpreter.h"
#include "Flint/Exceptions/RuntimeError.h"
#include "Flint/Callables/FlintCallable.h"
#include "Flint/Callables/Functions/FlintFunction.h"
#include "Flint/Callables/Classes/FlintInstance.h"
#include "Flint/Callables/Classes/FlintClass.h"
#include "Flint/FlintArray.h"
#include "Flint/Callables/Functions/BuiltInFunction.h"
#include "Flint/FlintString.h"

// ─────────────────────────────────────────────────────────────────────────────
// Binary Expression Evaluation
// Handles expressions like a + b, x > y, etc.
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
            else if (std::holds_alternative<std::shared_ptr<FlintString>>(left) || 
                std::holds_alternative<std::shared_ptr<FlintString>>(right))
            {
                // Convert left to string if it's not already
                std::string lstr = std::holds_alternative<std::shared_ptr<FlintString>>(left) 
                                   ? std::get<std::shared_ptr<FlintString>>(left)->value
                                   : Interpreter::stringify(left);  // fallback to number, bool, etc.

                // Convert right to string if it's not already
                std::string rstr = std::holds_alternative<std::shared_ptr<FlintString>>(right) 
                                   ? std::get<std::shared_ptr<FlintString>>(right)->value
                                   : Interpreter::stringify(right);

                return std::make_shared<FlintString>(lstr + rstr);
            }

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
                message = "divide by zero? seriously? who gave this kid a computer.";
                throw RuntimeError(expr.op, message);
            }
            return std::get<double>(left) / std::get<double>(right);

        case TokenType::MODULO:
            checkOperandType(expr.op, left, right);
            if (std::get<double>(right) == 0) {
                message = "divide by zero? seriously? who gave this kid a computer.";
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
// Ternary Conditional Evaluation: condition ? trueExpr : falseExpr
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
    if(auto strPtr = std::get_if<std::string>(&expr.value))
    {
        return std::make_shared<FlintString>(*strPtr);
    }
    return expr.value;
}

// ─────────────────────────────────────────────────────────────────────────────
// Grouping (i.e., parentheses): (a + b) just returns the inner value
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

    LiteralValue result = function->call(interpreter, arguments, expr.paren);
    return result;
}

LiteralValue Evaluator::operator()(const Get& expr) const
{
    LiteralValue val = evaluate(expr.object);

    // Handle string properties and methods
    if (auto strPtr = std::get_if<std::shared_ptr<FlintString>>(&val)) {
        return strPtr -> get() -> getInBuiltFunction(expr.name);
    }

    // Handle array properties and methods
    if (auto arrPtr = std::get_if<std::shared_ptr<FlintArray>>(&val)) {
        return arrPtr -> get() -> getInBuiltFunction(expr.name);
    }

    // Object/class/instance property access
    if (std::holds_alternative<std::shared_ptr<FlintCallable>>(val)) {
        auto classPtr = std::dynamic_pointer_cast<FlintClass>(std::get<std::shared_ptr<FlintCallable>>(val));
        if (classPtr)
            return classPtr->get(expr.name, interpreter);
    }
    else if (std::holds_alternative<std::shared_ptr<FlintInstance>>(val)) {
        return std::get<std::shared_ptr<FlintInstance>>(val)->get(expr.name, interpreter);
    }
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
    throw RuntimeError(expr.name, "Only instances, strings, or arrays have properties.");
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

LiteralValue Evaluator::operator()(const Super& expr, ExprPtr exprPtr) const
{
    int distance = interpreter.locals[exprPtr];
    std::shared_ptr<FlintClass> superClass = std::get<std::shared_ptr<FlintClass>>
        (interpreter.environment -> getAt(distance, expr.keyword));
    std::shared_ptr<FlintInstance> object = std::get<std::shared_ptr<FlintInstance>>
        (interpreter.environment -> getAt(distance - 1, 
            Token{ TokenType::THIS, "this", nullptr, 0 })
        );
    
    std::shared_ptr<FlintFunction> method = superClass -> findMethod(expr.method.lexeme);

    if (!method) {
      throw RuntimeError(expr.method,
          "Undefined property '" + expr.method.lexeme + "'.");
    }
    return method -> bind(object);
}

LiteralValue Evaluator::operator()(const Array& expr) const
{
    std::vector<LiteralValue> elements;

    for(auto& e : expr.elements) {
        elements.push_back(evaluate(e));
    }

    return std::make_shared<FlintArray>(std::move(elements));
}

LiteralValue Evaluator::evaluate(const ExprPtr& expr) const 
{
    if (!expr) return std::monostate{};

    LiteralValue result;
    std::visit([&](auto& actualExpr) {
        using T = std::decay_t<decltype(actualExpr)>;
        if constexpr (std::is_same_v<T, Variable> || 
            std::is_same_v<T, Assignment> || std::is_same_v<T, This> ||
            std::is_same_v<T, Super>)
            result = (*this)(actualExpr, expr);  // Pass ExprPtr
        else
            result = (*this)(actualExpr);        // Regular
    }, *expr);

    return result;
}

LiteralValue Evaluator::operator()(const GetIndex& expr) const
{
    LiteralValue arrVal = evaluate(expr.array);
    LiteralValue indexVal = evaluate(expr.index);

    if (auto arrPtr = std::get_if<std::shared_ptr<FlintArray>>(&arrVal))
    {
        checkOperandType(expr.bracket, indexVal);
        int index = static_cast<int>(std::get<double>(indexVal));
        if(index < 0 || index >= (int)(*arrPtr) -> elements.size())
            throw RuntimeError(expr.bracket, "Array index out of bounds \033[33m(why are you always reaching for things you can't have?)\033[0m");
        return (*arrPtr) -> elements[index];
    }

    if (auto strPtr = std::get_if<std::shared_ptr<FlintString>>(&arrVal))
    {
        checkOperandType(expr.bracket, indexVal);
        int index = static_cast<int>(std::get<double>(indexVal));
        if(index < 0 || index >= (int)(*strPtr) -> value.length())
            throw RuntimeError(expr.bracket, "String index out of bounds \033[33m(why are you always reaching for things you can't have?)\033[0m");
        return std::string(1, (*strPtr) -> value[index]);
    }

    throw RuntimeError(expr.bracket, "Only arrays or strings can be indexed.");
}

LiteralValue Evaluator::operator()(const SetIndex& expr) const
{
    auto arrVal = evaluate(expr.array);
    auto indexVal = evaluate(expr.index);
    auto newVal = evaluate(expr.value);

    if (auto arrPtr = std::get_if<std::shared_ptr<FlintArray>>(&arrVal)) {
        checkOperandType(expr.bracket, indexVal);
        int index = static_cast<int>(std::get<double>(indexVal));
        if(index < 0 || index >= (int)(*arrPtr) -> elements.size())
            throw RuntimeError(expr.bracket, "Array index out of bounds.");
        (*arrPtr)->elements[index] = newVal;
        return newVal;
    }
    throw RuntimeError(expr.bracket, "Only arrays support indexed assignment.");
}

// ─────────────────────────────────────────────────────────────────────────────
// Truthiness Evaluation
// Implements truth rules:
//   • false, null, and nothing → false
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

std::string Evaluator::getMethodName(const ExprPtr& callee) const {
    if (auto getExpr = std::get_if<Get>(callee.get())) {
        return getExpr->name.lexeme;
    }
    throw std::runtime_error("Method call is not in the expected format.");
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

    std::string message = "compiler is disappointed in you \033[33m(pls go touch grass)\033[0m";
    throw RuntimeError(op, message);
}