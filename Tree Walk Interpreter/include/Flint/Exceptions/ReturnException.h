#pragma once

#include <iostream>
#include "Flint/Parser/Value.h"

// A custom exception class used to signal a return statement being evaluated in a function.
class ReturnException : public std::runtime_error
{
public:
    // Holds the return value of the function.
    LiteralValue val;

    // Constructor takes a LiteralValue and initializes the base std::runtime_error with "Return".
    ReturnException(LiteralValue val)
        : std::runtime_error("Return"), val(val) {}
};