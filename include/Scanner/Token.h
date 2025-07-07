#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  Token Class
// ─────────────────────────────────────────────────────────────────────────────
//  Represents a single token in the source code, created by the Scanner.
//
//  Tokens are the smallest meaningful units in the Flint language.
//  Each token includes:
//    - Type: the token category (from TokenType enum)
//    - Lexeme: the exact substring from source code
//    - Literal: the parsed value (e.g., number, string)
//    - Line: line number for error reporting
//
//  Example:
//    Input:     let x = 42;
//    Tokens:    [LET, "let"], [IDENTIFIER, "x"], [EQUAL, "="], [NUMBER, "42"], [SEMICOLON, ";"]
// ─────────────────────────────────────────────────────────────────────────────

#include <string>
#include <iostream>
#include "Value.h"
#include "TokenType.h"

class Token
{
public:
    TokenType type;           // Type of the token (e.g., IDENTIFIER, PLUS, NUMBER)
    std::string lexeme;       // Raw substring from source (e.g., "let", "42", "+")
    LiteralValue literal;     // Optional evaluated value (e.g., double/string/bool/nil)
    size_t line;              // Line number in source (used for error messages)

    // ─────────────────────────────────────────────────────────────
    // Constructor
    // ─────────────────────────────────────────────────────────────
    Token(TokenType type, const std::string& lexeme, 
          const LiteralValue literal, int line)
        : type(type), lexeme(lexeme), literal(literal), line(line) {}

    // ─────────────────────────────────────────────────────────────
    // Debug Print
    // ─────────────────────────────────────────────────────────────
    // Converts token info to a string format for logging/debugging:
    // Example: IDENTIFIER x  nil  (line 2)
    std::string toString() const;
};