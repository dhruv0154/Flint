#pragma once

// Represents the type of function being declared or called.
// Used by the resolver and interpreter to handle scoping, 'this', and 'return' semantics.
enum class FunctionType
{
    NONE,         // Not currently inside any function.
    FUNCTION,     // A standard named function.
    LAMBDA,       // An anonymous lambda expression.
    METHOD,       // A method defined inside a class.
    INITIALIZER   // A special constructor method (used to initialize instances).
};
