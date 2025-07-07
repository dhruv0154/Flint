#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  Scanner (Lexer)
// ─────────────────────────────────────────────────────────────────────────────
//  Responsible for converting raw source code into a list of tokens.
//  This is the first step of the frontend pipeline.
//
//  This class implements a hand-written scanner (lexer) that recognizes:
//  - Operators (+, -, ==, etc.)
//  - Punctuators (parens, braces, semicolons)
//  - Keywords (let, print, etc.)
//  - Identifiers, strings, and numbers
//
//  Comments, whitespace, and invalid tokens are also handled here.
// ─────────────────────────────────────────────────────────────────────────────

#include <memory>
#include <vector>
#include <unordered_map>
#include "Token.h"
#include "Value.h"

class Scanner
{
public:
    // ─────────────────────────────────────────────────────────────
    // Constructor
    // ─────────────────────────────────────────────────────────────
    Scanner(const std::string& source);  // Initializes scanner with raw source code

    // ─────────────────────────────────────────────────────────────
    // Public Entry Point
    // ─────────────────────────────────────────────────────────────
    std::vector<Token> scanTokens();     // Tokenizes the entire source and returns list

private:
    // ─────────────────────────────────────────────────────────────
    // Internal State
    // ─────────────────────────────────────────────────────────────
    std::string source;  // The complete source code string
    std::vector<Token> tokens;  // Accumulated list of tokens
    static std::unordered_map<std::string, TokenType> keywords;  // Maps keyword strings to token types

    size_t start = 0;    // Start index of the current lexeme
    size_t current = 0;  // Current index being scanned
    size_t line = 1;     // Current line number (used for error reporting)

    // ─────────────────────────────────────────────────────────────
    // Main Tokenization Loop
    // ─────────────────────────────────────────────────────────────
    void scanToken();    // Scans one token at a time
    bool isAtEnd();      // True if we've reached the end of input

    // ─────────────────────────────────────────────────────────────
    // Character Handling Helpers
    // ─────────────────────────────────────────────────────────────
    char advance();      // Consumes and returns current character
    char peek();         // Returns current character without advancing
    char peekNext();     // Peeks one character ahead
    bool match(char expected);  // If next char matches, consume it

    // ─────────────────────────────────────────────────────────────
    // Character Classification
    // ─────────────────────────────────────────────────────────────
    bool isDigit(char c);         // 0–9
    bool isAlpha(char c);         // a–z, A–Z, _
    bool isAlphaNumeric(char c);  // isAlpha || isDigit

    // ─────────────────────────────────────────────────────────────
    // Token Emitters
    // ─────────────────────────────────────────────────────────────
    void addToken(TokenType type);                        // Token without literal
    void addToken(TokenType type, LiteralValue literal);  // Token with literal (e.g. number, string)

    // ─────────────────────────────────────────────────────────────
    // Lexeme Handlers
    // ─────────────────────────────────────────────────────────────
    void string();      // Handles string literals (enclosed in double quotes)
    void number();      // Handles numeric literals (integers, decimals)
    void identifier();  // Handles identifiers and reserved keywords
    void blockComments(); // Handles nested block comments (/* ... */)
};