#pragma once

#include <iostream>
#include "Scanner/Token.h"

class RuntimeError  : public std::runtime_error
{
public:
    Token token;
    RuntimeError(Token token, std::string& messsage) : std::runtime_error(messsage), token(token) {}
};