#include "Interpreter/Interpreter.h"
#include "ExpressionNode.h"
#include "RuntimeError.h"
#include "Flint/Flint.h"

void Interpreter::interpret(ExpressionNode expr) const
{
    try
    {
        std::unique_ptr<Evaluator> evl = std::make_unique<Evaluator>();
        LiteralValue result = evl -> evaluate(expr);
        std::cout << stringify(result);
    }
    catch (RuntimeError error)
    {
        Flint::runtimeError(error);
    }
}

std::string Interpreter::stringify(const LiteralValue& obj)
{
    auto func = [] (auto&& val) -> std::string 
    {
        using T = std::decay_t<decltype(val)>;

        if constexpr (std::is_same_v<T, nullptr_t> || std::is_same_v<T, std::monostate>) return "NOTHING";
        else if constexpr (std::is_same_v<T, bool>) return val ? "true" : "false";
        else if constexpr (std::is_same_v<T, std::string>) return val;
        else if constexpr (std::is_same_v<T, double>)
        {
            std::string text = std::to_string(val);
            text.erase(text.find_last_not_of('0') + 1);
            if(text.back() == '.') text.pop_back();
            return text;
        }
        else return std::to_string(val);  
    };

    return std::visit(func, obj);
}