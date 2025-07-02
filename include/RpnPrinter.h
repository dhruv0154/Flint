#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <variant>
#include "ExpressionNode.h"

// Prints expressions in Reverse Polish Notation (RPN)
struct RpnPrinter
{
public:
    std::string operator()(const Binary& expr)
    {
       return convertToRpn(expr.op.lexeme, expr.left.get(), expr.right.get());
    }

    std::string operator()(const Grouping& expr)
    {
       return convert(*expr.expression);
    }

    std::string operator()(const Unary& expr)
    {
        return convertToRpn(expr.op.lexeme, expr.right.get());
    }

    std::string operator()(const Literal& expr)
    {
        return std::visit([](auto&& val) -> std::string {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, std::monostate>) return "nil";
        else if constexpr (std::is_same_v<T, int>) return std::to_string(val);
        else if constexpr (std::is_same_v<T, double>) return std::to_string(val);
        else if constexpr (std::is_same_v<T, std::string>) return val;
        else if constexpr (std::is_same_v<T, std::nullptr_t>) return "null";
        else if constexpr (std::is_same_v<T, bool>) return val ? "true" : "false";
        else return "unknown";
        }, expr.value);
    }
    std::string convert(const ExpressionNode& a)
    {
        return std::visit(*this, a);
    }
    
private:
    std::string convertToRpn(const std::string& name, ExpressionNode* a, ExpressionNode* b = nullptr) {
        std::string output;
        output.append(convert(*a) + " ");
        if (b) output.append(convert(*b) + " ");
        output.append(name + " ");
        return output;
    }
};