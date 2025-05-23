#pragma once

#include <string>
#include <iostream>
#include <any>
#include "TokenType.h"

class Token
{
    public:
        TokenType type;
        std::string lexeme;
        std::any literal;
        int line;

        Token(TokenType type, const std::string& lexeme, const std::any literal, int line)
        : type(type), lexeme(lexeme), literal(literal), line(line) {}

        std::string toString() const;
        
};