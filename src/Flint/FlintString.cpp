#include <algorithm>
#include "Flint/FlintString.h"
#include "Flint/Interpreter/Interpreter.h"
#include "Flint/Exceptions/RuntimeError.h"

FlintString::FlintString(std::string value)
  : value(std::move(value))
{

    builtInFunctions["lower"] = std::make_shared<BuiltinFunction>(
        [this](Interpreter&,
                   const std::vector<LiteralValue>& args,
                   const Token& token
                 ) -> LiteralValue {
            if (!args.empty())
                throw RuntimeError(token, "lower takes no arguments");
            std::transform(
                this->value.begin(), this->value.end(),
                this->value.begin(),
                ::tolower
            );
            // wrap the mutated string back into a LiteralValue
            return LiteralValue(shared_from_this());
        },
        /*arity=*/0
    );

    builtInFunctions["upper"] = std::make_shared<BuiltinFunction>(
        [this](Interpreter&,
                   const std::vector<LiteralValue>& args,
                   const Token& token
                 ) -> LiteralValue {
            if (!args.empty())
                throw RuntimeError(token, "upper takes no arguments");
            std::transform(
                this->value.begin(), this->value.end(),
                this->value.begin(),
                ::toupper
            );
            return LiteralValue(shared_from_this());
        },
        0
    );

    builtInFunctions["length"] = std::make_shared<BuiltinFunction>(
        [this](Interpreter&,
                   const std::vector<LiteralValue>& args,
                   const Token& token
                 ) -> LiteralValue {
            if (!args.empty())
                throw RuntimeError(token, "length takes no arguments");
            // return a number wrapped as a LiteralValue
            return LiteralValue(static_cast<double>(this->value.length()));
        },
        0
    );
}
LiteralValue FlintString::getInBuiltFunction(Token name)
{
    if(builtInFunctions.count(name.lexeme)) return builtInFunctions[name.lexeme];
    throw RuntimeError(name, "string has no function " + name.lexeme + ".");
    return nullptr;
}