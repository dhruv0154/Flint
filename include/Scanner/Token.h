#pragma once

#include <string>
#include <iostream>
#include "Value.h"
#include "TokenType.h"

class Token
{
public:
    TokenType type;   // Type of the token (e.g., IDENTIFIER, PLUS, NUMBER)
    std::string lexeme;   // The actual source text of the token
    LiteralValue literal; // Optional literal value
    size_t line;   // Line number where this token appears

    // Constructs a token with all metadata
    Token(TokenType type, const std::string& lexeme, 
          const LiteralValue literal, int line)
        : type(type), lexeme(lexeme), literal(literal), line(line) {}

    // Converts the token to a printable string (for debugging or REPL)
    std::string toString() const;
};