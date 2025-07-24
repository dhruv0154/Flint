#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  Token.h – Lexical Token Representation for Flint
// ─────────────────────────────────────────────────────────────────────────────
//  Declares the Token class, the fundamental unit output by the Scanner.
//  Each Token encapsulates:
//    - type: category from TokenType (keywords, operators, literals, etc.)
//    - lexeme: the exact substring from the source code
//    - literal: parsed runtime value for literal tokens (numbers, strings, bool, nil)
//    - line: source line number for error reporting
//
//  Used by Parser to recognize grammar constructs and by Interpreter for
//  error diagnostics and literal evaluation.
// ─────────────────────────────────────────────────────────────────────────────

#include <string>
#include "Value.h"       // Defines LiteralValue: variant of supported literal types
#include "TokenType.h"  // Enumerates all token categories (IDENTIFIER, NUMBER, PLUS, etc.)

class Token {
public:
    TokenType type;        // Token category (e.g., IDENTIFIER, PLUS, NUMBER)
    std::string lexeme;    // Exact source substring (e.g., "let", "x", "42")
    LiteralValue literal;  // Evaluated literal value; unused for non-literals
    size_t line;           // Line number in source text (for error messages)

    //──────────────────────────────────────────────────────────────────────────
    // Constructor: initialize all token fields
    //──────────────────────────────────────────────────────────────────────────
    Token(TokenType type,
          const std::string& lexeme,
          LiteralValue literal,
          int line)
        : type(type)
        , lexeme(lexeme)
        , literal(literal)
        , line(line)
    {}

    //──────────────────────────────────────────────────────────────────────────
    // toString: human-readable representation for debugging
    //──────────────────────────────────────────────────────────────────────────
    // Example output: "IDENTIFIER x nil (line 2)"
    std::string toString() const;
};