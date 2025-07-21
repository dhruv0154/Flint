#include "FlintFunction.h"

LiteralValue FlintFunction::call(Interpreter &interpreter, 
        const std::vector<LiteralValue> &args, const Token &paren)
{
    std::shared_ptr<Environment> environment = std::make_shared<Environment>(closure);
    for(size_t i = 0; i < declaration -> params.size(); i++)
    {
        environment -> define(declaration -> params.at(i).lexeme, args.at(i));
    }
    try
    {
        interpreter.executeBlock(declaration -> body, environment);
    }
    catch(const ReturnException &e)
    {
        if(isInitializer) 
            return closure -> getAt(0, Token{TokenType::IDENTIFIER, "this", nullptr, 0});
        return e.val;
    }

    if(isInitializer) return closure -> getAt(0, Token{TokenType::IDENTIFIER, "this", nullptr, 0});
    return nullptr;
}

int FlintFunction::arity() const { return declaration -> params.size(); }

std::string FlintFunction::toString() const
{
    if(declaration -> name.has_value())
        return "<fn " + declaration -> name -> lexeme + ">";
    else
        return "<lambda>";
}
LiteralValue FlintFunction::bind(LiteralValue instance)
{
    std::shared_ptr<Environment> environment = std::make_shared
        <Environment>(closure);
    environment -> define("this", instance);
    return std::make_shared<FlintFunction>(declaration, environment, isInitializer);
}