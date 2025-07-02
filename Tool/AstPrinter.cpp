#include "AstPrinter.h"
#include "Evaluator.h"


using Value = std::variant<std::monostate, std::string, double, int>; // Represents different literal types


int main(int argc, char const *argv[])
{
    using ExprPtr = std::shared_ptr<ExpressionNode>;

    // Represents: (-123)
    ExprPtr unaryExpr = std::make_shared<ExpressionNode>(
        Unary{
        Token(TokenType::MINUS, "-", nullptr, 1),
        std::make_shared<ExpressionNode>(Literal{123.0} 
)
        }
    );

    // Represents: (group 45.67)
    ExprPtr groupingExpr = std::make_shared<ExpressionNode>(
        Grouping{
        std::make_shared<ExpressionNode>(Literal{45.67} 
)
        }
    );

    // Represents: (-123) * (group 45.67)
    ExprPtr expression = std::make_shared<ExpressionNode>(
        Binary
        {
            unaryExpr,
            Token(TokenType::STAR, "*", nullptr, 1),
            groupingExpr
        }
    );

    // ExprPtr expression = std::make_shared<ExpressionNode>(Binary{
    //     std::make_shared<ExpressionNode>(Unary{
    //         Token{TokenType::MINUS, "-", nullptr, 1},
    //         std::make_shared<ExpressionNode>(Literal{123.0})
    //     }),
    //     Token{TokenType::PLUS, "+", nullptr, 1},
    //     std::make_shared<ExpressionNode>(Grouping{
    //         std::make_shared<ExpressionNode>(Literal{45.67})
    //     })
    // });

    // Print in Lisp-style
    AstPrinter printer;
    Evaluator evaluator;

    std::cout << "AST: " << printer.print(*expression) << std::endl;
    std::cout << "Result: " << std::get<double>(evaluator.evaluate(*expression)) << std::endl;
    return 0;
}