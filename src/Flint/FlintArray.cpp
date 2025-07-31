#include "Flint/FlintArray.h"
#include "Flint/Exceptions/RuntimeError.h"

FlintArray::FlintArray(std::vector<LiteralValue> elems) : elements(std::move(elems))
{
    builtInFunctions["push"] = std::make_shared<BuiltinFunction>([this](Interpreter&,
                 const std::vector<LiteralValue>& args, const Token& token) {
                if (args.size() != 1)
                    throw RuntimeError(token, "push() takes exactly one argument.");
                (this)->elements.push_back(args[0]);
                return nullptr;
            }, 1);
    
    builtInFunctions["pop"] = std::make_shared<BuiltinFunction>([this](Interpreter&, 
                const std::vector<LiteralValue>& args, const Token& token) {
                
                if (!args.empty()) 
                    throw RuntimeError(token, "pop() takes no arguments.");
                if (this->elements.empty())
                    throw RuntimeError(token, "Cannot pop from empty array.");
                auto val = this->elements.back();
                this->elements.pop_back();
                return val;
            }, 0);

    builtInFunctions["length"] = std::make_shared<BuiltinFunction>([this](Interpreter&, 
            const std::vector<LiteralValue>& args, const Token& token) {

                if(!args.empty()) 
                    throw RuntimeError(token, "length() takes no arguments.");
                return LiteralValue(static_cast<double>(this -> elements.size()));
            }, 0);
}

LiteralValue FlintArray::getInBuiltFunction(Token name)
{
    if (builtInFunctions.count(name.lexeme)) return builtInFunctions[name.lexeme];
    throw RuntimeError(name, "array has no function named " + name.lexeme + ".");
}