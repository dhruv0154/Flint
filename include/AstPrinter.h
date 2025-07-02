
#pragma once

#include "ExpressionNode.h"

struct AstPrinter {
    std::string operator()(const Binary& expr) const {
        return parenthesize(expr.op.lexeme, *expr.left, *expr.right);
    }

    std::string operator()(const Unary& expr) const {
        return parenthesize(expr.op.lexeme, *expr.right);
    }

    std::string operator()(const Literal& expr) const {
        if (std::holds_alternative<std::monostate>(expr.value)) return "nil";
        if (std::holds_alternative<int>(expr.value)) return std::to_string(std::get<int>(expr.value));
        if (std::holds_alternative<double>(expr.value)) return std::to_string(std::get<double>(expr.value));
        if (std::holds_alternative<std::string>(expr.value)) return std::get<std::string>(expr.value);
        if (std::holds_alternative<std::nullptr_t>(expr.value)) return "null";
        if (std::holds_alternative<bool>(expr.value)) return std::get<bool>(expr.value) ? "true" : "false";
        return "unknown";
    }

    std::string operator()(const Grouping& expr) const {
        return parenthesize("group", *expr.expression);
    }

    std::string print(const ExpressionNode& expr) const {
        return std::visit(*this, expr);
    }

private:
    template <typename... Exprs>
    std::string parenthesize(const std::string& name, const Exprs&... exprs) const {
        std::string result = "(" + name;
        ((result += " " + print(exprs)), ...);
        result += ")";
        return result;
    }
};