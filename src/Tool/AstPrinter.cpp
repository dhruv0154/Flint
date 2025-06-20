#include <string>
#include <any>
#include <sstream>
#include <iomanip>
#include <variant>
#include <format>
#include "C:\Flint\include\Scanner\generated\Expr.h"

using Value = std::variant<std::monostate, std::string, double, int>;

class AstPrinter : public Visitor<std::string> 
{
    public:
        std::string visitBinaryExpr(Binary<std::string>& expr) override {
            return parenthesize(expr.op.lexeme, expr.left.get(), expr.right.get());
        }

        std::string visitGroupingExpr(Grouping<std::string>& expr) override {
            return parenthesize("group", expr.expression.get());
        }

        std::string visitLiteralExpr(Literal<std::string>& expr) override 
        {
            return std::visit([](auto&& val) -> std::string 
            { 
                using T = std::decay_t<decltype(val)>;

                if constexpr (std::is_same_v<T, std::monostate>) return "nil"; 
                else if constexpr (std::is_same_v<T, std::string>) return val;
                else if constexpr (std::is_same_v<T, double>) 
                {
                    std::ostringstream ss;
                    ss << std::fixed << std::setprecision(2) << val;
                    return ss.str();
                }
                else if constexpr (std::is_same_v<T, int>) 
                {
                   return std::to_string(val);
                }
                else return "unknown";
                
            }, expr.value);
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

class RpnPrinter : public Visitor<std::string>
{
    public:
        std::string visitBinaryExpr(Binary<std::string>& expr) override
        {
            return convertToRpn(expr.op.lexeme, expr.left.get(), expr.right.get());
        }

        std::string visitGroupingExpr(Grouping<std::string>& expr) override
        {
            return expr.expression -> accept(*this);
        }

        std::string visitLiteralExpr(Literal<std::string>& expr) override 
        {
            return std::visit([](auto&& val) -> std::string 
            { 
                using T = std::decay_t<decltype(val)>;

                if constexpr (std::is_same_v<T, std::monostate>) return "nil"; 
                else if constexpr (std::is_same_v<T, std::string>) return val;
                else if constexpr (std::is_same_v<T, double>) 
                {
                    std::ostringstream ss;
                    ss << std::fixed << std::setprecision(2) << val;
                    return ss.str();
                }
                else if constexpr (std::is_same_v<T, int>) 
                {
                    return std::to_string(val);
                }
                else return "unknown";

            }, expr.value);
        }

        std::string visitUnaryExpr(Unary<std::string>& expr) override 
        {
            return convertToRpn(expr.op.lexeme, expr.right.get());
        }

    private:
        std::string convertToRpn(const std::string& name, Expr<std::string>* a, Expr<std::string>* b = nullptr)
        {
            std::string output;
            output.append(a->accept(*this) + " ");
            if(b) output.append(b->accept(*this) + " ");
            output.append(name + " ");
            return output;
        }
};

int main(int argc, char const *argv[])
{
    using ExprPtr = std::shared_ptr<Expr<std::string>>;

    // Build: (-123)
    ExprPtr unaryExpr = std::make_shared<Unary<std::string>>(
        Token(TokenType::MINUS, "-", nullptr, 1),
        std::make_shared<Literal<std::string>>(123.0)
    );

    // Build: (group 45.67)
    ExprPtr groupingExpr = std::make_shared<Grouping<std::string>>(
        std::make_shared<Literal<std::string>>(45.67)
    );

    // Combine into: (-123) * (group 45.67)
    ExprPtr expression = std::make_shared<Binary<std::string>>(
        unaryExpr,
        Token(TokenType::STAR, "*", nullptr, 1),
        groupingExpr
    );

    // Print the expression
    AstPrinter printer;
    std::cout << expression -> accept(printer) << std::endl;

    // expression = std::make_shared<Binary<std::string>>(std::make_shared<Literal<std::string>>(123), 
    // Token(TokenType::STAR, "*", nullptr, 1) , std::make_shared<Literal<std::string>>(45));

    RpnPrinter rpn;

    std::cout << expression -> accept(rpn) << "\n";
    return 0;
}