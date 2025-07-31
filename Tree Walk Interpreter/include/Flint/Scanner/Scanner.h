#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  Scanner.h – Lexical Analysis for Flint
// ─────────────────────────────────────────────────────────────────────────────
//  Converts raw source code into a sequence of Tokens for parsing.
//  Recognizes:
//    - Single- and multi-character operators (e.g., +, -, ==)
//    - Punctuation (parentheses, braces, semicolons)
//    - Literals (identifiers, strings, numbers)
//    - Keywords (let, print, if, etc.)
//  Handles whitespace, comments, and reports invalid tokens.
// ─────────────────────────────────────────────────────────────────────────────

#include <string>
#include <vector>
#include <unordered_map>
#include "Token.h"   // Token struct: type, lexeme, literal, line
#include "Flint/Parser/Value.h"   // LiteralValue variant for number/string literals

class Scanner {
public:
    //──────────────────────────────────────────────────────────────────────────
    // Constructor
    //──────────────────────────────────────────────────────────────────────────
    // @param source: full source code as a single string
    explicit Scanner(const std::string& source);

    //──────────────────────────────────────────────────────────────────────────
    // scanTokens
    //──────────────────────────────────────────────────────────────────────────
    // Performs the full scanning pass and returns the list of Tokens.
    std::vector<Token> scanTokens();

private:
    //──────────────────────────────────────────────────────────────────────────
    // Core state
    //──────────────────────────────────────────────────────────────────────────
    std::string source;                        // Source text
    std::vector<Token> tokens;                 // Accumulated tokens
    static std::unordered_map<std::string, TokenType> keywords;  // Keyword lookup

    size_t start = 0;    // Start of current lexeme
    size_t current = 0;  // Current position in source
    size_t line = 1;     // Current line number for error reporting

    //──────────────────────────────────────────────────────────────────────────
    // scanToken
    //──────────────────────────────────────────────────────────────────────────
    // Scans a single token from the source at `current`.
    void scanToken();

    //──────────────────────────────────────────────────────────────────────────
    // Character utilities
    //──────────────────────────────────────────────────────────────────────────
    bool isAtEnd() const;            // True when `current >= source.length()`
    char advance();                  // Consume and return next character
    char peek() const;               // Look at current char without consuming
    char peekNext() const;           // Look ahead one character
    bool match(char expected);       // If next char equals expected, consume it

    //──────────────────────────────────────────────────────────────────────────
    // Character classification
    //──────────────────────────────────────────────────────────────────────────
    bool isDigit(char c) const;      // Checks '0'–'9'
    bool isAlpha(char c) const;      // Checks 'a'–'z', 'A'–'Z', and '_'
    bool isAlphaNumeric(char c) const; // isAlpha || isDigit

    //──────────────────────────────────────────────────────────────────────────
    // Token emission
    //──────────────────────────────────────────────────────────────────────────
    void addToken(TokenType type);                          // No literal
    void addToken(TokenType type, LiteralValue literal);    // With literal value

    //──────────────────────────────────────────────────────────────────────────
    // Lexeme handlers
    //──────────────────────────────────────────────────────────────────────────
    void string();        // Process string literal until closing '"'
    void number();        // Process integer or floating-point literal
    void identifier();    // Process identifier or keyword
    void blockComments(); // Skip over nested /* ... */ comments
};