#pragma once

#include <string>
#include <iostream>
#include <variant>
#include "TokenType.h"

class Token
{
    public:
        TokenType type;
        std::string lexeme;
        std::variant<std::monostate, std::string, int, double, std::nullptr_t> literal;
        size_t line;

        Token(TokenType type, const std::string& lexeme, 
            const std::variant<std::monostate, std::string, int, double, std::nullptr_t> literal, int line)
        : type(type), lexeme(lexeme), literal(literal), line(line) {}

        std::string toString() const;
        
};