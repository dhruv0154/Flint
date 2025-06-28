#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include "Token.h"
#include "Value.h"

class Scanner
{
public:
    Scanner(const std::string& source); // Initialize with source code
    std::vector<Token> scanTokens();    // Tokenize entire source

private:
    std::string source;                         // Complete input source code
    std::vector<Token> tokens;                  // List of generated tokens
    static std::unordered_map<std::string, TokenType> keywords; // Reserved keywords mapping
    size_t start = 0;                           // Start index of current lexeme
    size_t current = 0;                         // Index of character being processed
    size_t line = 1;                            // Line number (for error reporting)

    // High-level scanning
    void scanToken();                           // Process one token
    bool isAtEnd();                             // Check if we’ve reached the end

    // Character inspection
    char advance();                             // Consume and return current char
    char peek();                                // Look at current char without advancing
    char peekNext();                            // Look two characters ahead
    bool match(char expected);                  // Conditionally consume next char if matched

    // Classification
    bool isDigit(char c);                       // Check if char is a digit
    bool isAlpha(char c);                       // Check if char is alphabet or '_'
    bool isAlphaNumeric(char c);                // Check if char is alphanumeric or '_'

    // Token creators
    void addToken(TokenType type);              // Add token without literal
    void addToken(TokenType type, LiteralValue literal); // Add token with literal

    // Lexeme handlers
    void string();                              // Handle string literals
    void number();                              // Handle number literals
    void identifier();                          // Handle identifiers and keywords
    void blockComments();                       // Handle nested /* */ comments
};