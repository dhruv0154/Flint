#include <string>
#include <any>
#include <sstream>
#include "C:\Flint\include\Scanner\generated\Expr.h"

class AstPrinter : public Visitor<std::string> {
public:
    std::string print(Expr<std::string>* expr) {
        return expr->accept(*this);
    }

    std::string visitBinaryExpr(Binary<std::string>& expr) override {
        return parenthesize(expr.op.lexeme, expr.left.get(), expr.right.get());
    }

    std::string visitGroupingExpr(Grouping<std::string>& expr) override {
        return parenthesize("group", expr.expression.get());
    }

    std::string visitLiteralExpr(Literal<std::string>& expr) override {
        if (!expr.value.has_value()) return "nil";

        try {
            return std::any_cast<std::string>(expr.value);
        } catch (...) {
            try {
                return std::to_string(std::any_cast<double>(expr.value));
            } catch (...) {
                return "unknown";
            }
        }
    }

    std::string visitUnaryExpr(Unary<std::string>& expr) override {
        return parenthesize(expr.op.lexeme, expr.right.get());
    }

private:
    std::string parenthesize(const std::string& name, Expr<std::string>* a) {
        return "(" + name + " " + a->accept(*this) + ")";
    }

    std::string parenthesize(const std::string& name, Expr<std::string>* a, Expr<std::string>* b) {
        return "(" + name + " " + a->accept(*this) + " " + b->accept(*this) + ")";
    }
};

int main(int argc, char const *argv[])
{
    using ExprPtr = std::shared_ptr<Expr<std::string>>;

    // Build: (-123)
    ExprPtr unaryExpr = std::make_shared<Unary<std::string>>(
        Token(TokenType::MINUS, "-", std::any(), 1),
        std::make_shared<Literal<std::string>>(std::any(123.0))
    );

    // Build: (group 45.67)
    ExprPtr groupingExpr = std::make_shared<Grouping<std::string>>(
        std::make_shared<Literal<std::string>>(std::any(45.67))
    );

    // Combine into: (-123) * (group 45.67)
    ExprPtr expression = std::make_shared<Binary<std::string>>(
        unaryExpr,
        Token(TokenType::STAR, "*", std::any(), 1),
        groupingExpr
    );

    // Print the expression
    AstPrinter printer;
    std::cout << printer.print(expression.get()) << std::endl;
    return 0;
}