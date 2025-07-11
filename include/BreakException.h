#pragma once

#include <iostream>

class BreakException : std::runtime_error
{
public:
    BreakException() : std::runtime_error("Break") {}
};