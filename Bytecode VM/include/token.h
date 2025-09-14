#pragma once

#include <string_view>
#include "tokenType.h"

class Token
{ 
public:
    TokenType type;
    std::string_view lexeme;
    size_t line;
    Token(TokenType type, std::string_view lexeme, size_t line) : type(type), 
        lexeme(lexeme), line(line) {}
    Token() = default;
    ~Token() = default;
};