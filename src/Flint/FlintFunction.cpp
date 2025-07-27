#include "Flint/Callables/Functions/FlintFunction.h"

// Executes the function body and returns the result
LiteralValue FlintFunction::call(Interpreter &interpreter, 
        const std::vector<LiteralValue> &args, const Token &paren)
{
    // Create a new environment enclosing the closure (the environment where the function was defined)
    std::shared_ptr<Environment> environment = std::make_shared<Environment>(closure);
    
    // Define parameters in the new environment with corresponding argument values
    for(size_t i = 0; i < declaration -> params.size(); i++)
    {
        environment -> define(declaration -> params.at(i).lexeme, args.at(i));
    }

    try
    {
        // Execute the function body in the new environment
        interpreter.executeBlock(declaration -> body, environment);
    }
    catch(const ReturnException &e)
    {
        // If this is an initializer, always return 'this' regardless of explicit return
        if(isInitializer) 
            return closure -> getAt(0, Token{TokenType::IDENTIFIER, "this", nullptr, 0});
        // Return the returned value if normal function
        return e.val;
    }

    // If no return was encountered and it's an initializer, return 'this'
    if(isInitializer) return closure -> getAt(0, Token{TokenType::IDENTIFIER, "this", nullptr, 0});
    
    // Otherwise return null (i.e., no explicit return value)
    return nullptr;
}

// Returns the number of parameters the function expects
int FlintFunction::arity() const { return declaration -> params.size(); }

// String representation of the function (used for debugging or printing)
std::string FlintFunction::toString() const
{
    if(declaration -> name.has_value())
        return "<fn " + declaration -> name -> lexeme + ">";
    else
        return "<lambda>";  // anonymous function
}

// Binds the function to an instance by injecting 'this' into the environment
LiteralValue FlintFunction::bind(LiteralValue instance)
{
    // Create new environment enclosing the original closure
    std::shared_ptr<Environment> environment = std::make_shared<Environment>(closure);
    
    // Define 'this' in the new environment to point to the instance
    environment -> define("this", instance);

    // Return a new FlintFunction with the bound environment
    return std::make_shared<FlintFunction>(declaration, environment, isInitializer);
}