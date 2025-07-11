#pragma once

#include <iostream>

class ContinueException : public std::runtime_error
{

public:
    ContinueException() : std::runtime_error("Continue") {}
};