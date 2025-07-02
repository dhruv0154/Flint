#include "Evaluator.h"
#include "Interpreter/Interpreter.h"
#include "RuntimeError.h"

LiteralValue Evaluator::operator()(const Binary& expr) const 
{
    LiteralValue left = evaluate(expr.left);
    LiteralValue right = evaluate(expr.right);
    std::string message;

    switch (expr.op.type) {
        case TokenType::PLUS:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
                return std::get<double>(left) + std::get<double>(right);
            if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
                return std::get<std::string>(left) + std::get<std::string>(right);
            else if (std::holds_alternative<std::string>(left) && std::holds_alternative<double>(right))
                return std::get<std::string>(left) + Interpreter::stringify(std::get<double>(right));
            message = "Invalid operand type, Operand must be a number or string.";
            throw RuntimeError(expr.op, message);
        case TokenType::MINUS:
            checkOperandType(expr.op, left, right);
            return std::get<double>(left) - std::get<double>(right);
        case TokenType::STAR:
            checkOperandType(expr.op, left, right);
            return std::get<double>(left) * std::get<double>(right);
        case TokenType::SLASH:
            checkOperandType(expr.op, left, right);
            if(std::get<double>(right) == 0)
            {
                message = "Division by zero is undefined.";
                throw RuntimeError(expr.op, message);
            }
            return std::get<double>(left) / std::get<double>(right);
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
        case TokenType::BANG_EQUAL:
            return !isEqual(left, right);
        case TokenType::EQUAL_EQUAL:
            return isEqual(left, right);
        default:
            break;
    }

    return std::monostate{};
}

LiteralValue Evaluator::operator()(const Conditional& expr) const 
{
    LiteralValue condition = evaluate(expr.condition);
    return isTruthy(condition) ? evaluate(expr.left) : evaluate(expr.right);
}

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

LiteralValue Evaluator::operator()(const Literal& expr) const 
{
    return expr.value;
}

LiteralValue Evaluator::operator()(const Grouping& expr) const 
{
    return evaluate(expr.expression);
}

LiteralValue Evaluator::evaluate(const ExpressionNode& expr) const 
{
    return std::visit(*this, expr);
}

LiteralValue Evaluator::evaluate(const std::shared_ptr<ExpressionNode>& ptr) const 
{
    if (!ptr)
        return std::monostate{};
    return evaluate(*ptr);
}

bool Evaluator::isTruthy(const LiteralValue& value) const 
{
    auto visitor = [] (auto&& val) -> bool
    {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, std::monostate>)  { return false; }
        else if constexpr (std::is_same_v<T, std::nullptr_t>) { return false; }
        else if constexpr (std::is_same_v<T, bool>) { return val; }
        else if constexpr (std::is_same_v<T, double>) { return val != 0.0; }
        else if constexpr (std::is_same_v<T, int>) { return val != 0; }
        else if constexpr (std::is_same_v<T, std::string>) { return !val.empty(); }
    };

    return std::visit(visitor, value);
}

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

template<typename... Operands>
void Evaluator::checkOperandType(const Token& op, const Operands&... operands) const
{
    // Check if all operands hold a double
    if ((... && std::holds_alternative<double>(operands))) return;

    std::string message = "Invalid operand type, operands must be numbers.";
    throw RuntimeError(op, message);
}
