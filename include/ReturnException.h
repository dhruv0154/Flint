#pragma once

#include <iostream>
#include "Scanner/Value.h"

class ReturnException : public std::runtime_error
{
public:
    LiteralValue val;
    ReturnException(LiteralValue val) : std::runtime_error("Return"), val(val) {}

};
