#include "FlintInstance.h"
#include "FlintClass.h"
#include "RuntimeError.h"
#include "FlintFunction.h"

// Convert the instance to string representation (e.g., "MyClass instance")
std::string FlintInstance::toString() const {
    return klass->toString() + " instance";
}

// Access a field or method from the instance
LiteralValue FlintInstance::get(Token name, Interpreter& interpreter)
{
    // Check if the requested property is a field of the instance
    if (fields.count(name.lexeme)) 
        return fields[name.lexeme];

    // Otherwise, check if it's a method in the class
    LiteralValue method = klass->findMethod(name.lexeme);

    // Ensure the method is a callable type
    if (std::holds_alternative<std::shared_ptr<FlintCallable>>(method)) 
    {
        auto callable = std::get<std::shared_ptr<FlintCallable>>(method);
        if (callable) 
        {
            // Try to cast the callable to a FlintFunction
            auto fn = std::dynamic_pointer_cast<FlintFunction>(callable);
            if (fn) 
            {
                // Bind the method to the current instance
                auto bound = fn->bind(shared_from_this());

                // If it's a getter (zero-arg method called like a field)
                if (fn->declaration->isGetter) 
                {
                    // `bound` is a LiteralValue; extract the callable and invoke it immediately
                    auto boundFn = std::get<std::shared_ptr<FlintCallable>>(bound);
                    return boundFn->call(interpreter, {}, name);
                }

                // Return the bound method for normal access (without calling it yet)
                return bound;
            }

            // If it's some other type of callable, return as-is
            return callable;
        }
    }

    // If neither field nor method is found, throw a runtime error
    throw RuntimeError(name, "Undefined property '" + name.lexeme + "'.");
}

// Set or define a field on the instance
void FlintInstance::set(Token name, LiteralValue object)
{
    fields[name.lexeme] = object;
}