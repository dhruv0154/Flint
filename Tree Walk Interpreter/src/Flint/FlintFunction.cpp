#include "Flint/Callables/Functions/FlintFunction.h"

// Executes the function body and returns the result
LiteralValue FlintFunction::call(Interpreter &interpreter, 
        const std::vector<LiteralValue> &args, const Token &paren)
{
    // Create a new environment enclosing the closure (the environment where the function was defined)
    std::shared_ptr<Environment> environment = std::make_shared<Environment>(closure);
    
    // Define parameters in the new environment with corresponding argument values
    for (size_t i = 0; i < declaration->params.size(); ++i) {
        environment->define(declaration->params.at(i).lexeme, args.at(i));
    }

    // Prepare interpreter flags for this call (clear any previous state)
    interpreter.returning = false;
    interpreter.returnValue = nullptr;
    interpreter.breaking = false;
    interpreter.continuing = false;

    // Execute the function body in the new environment
    interpreter.executeBlock(declaration->body, environment);

    // If a return was signaled, capture it and clear the flag
    if (interpreter.returning) {
        LiteralValue rv = interpreter.returnValue;
        interpreter.returning = false;
        interpreter.returnValue = nullptr;

        if (isInitializer) {
            // Initializers always return 'this'
            return closure->getAt(0, Token{TokenType::IDENTIFIER, "this", nullptr, 0});
        }

        return rv;
    }

    // If no return was encountered and it's an initializer, return 'this'
    if (isInitializer) {
        return closure->getAt(0, Token{TokenType::IDENTIFIER, "this", nullptr, 0});
    }

    // Otherwise return null (i.e., no explicit return value)
    return nullptr;
}

// Returns the number of parameters the function expects
int FlintFunction::arity() const { return static_cast<int>(declaration->params.size()); }

// String representation of the function (used for debugging or printing)
std::string FlintFunction::toString() const
{
    if (declaration->name.has_value())
        return "<fn " + declaration->name->lexeme + ">";
    else
        return "<lambda>";  // anonymous function
}

// Binds the function to an instance by injecting 'this' into the environment
LiteralValue FlintFunction::bind(LiteralValue instance)
{
    // Create new environment enclosing the original closure
    std::shared_ptr<Environment> environment = std::make_shared<Environment>(closure);
    
    // Define 'this' in the new environment to point to the instance
    environment->define("this", instance);

    // Return a new FlintFunction with the bound environment
    return std::make_shared<FlintFunction>(declaration, environment, isInitializer);
}
