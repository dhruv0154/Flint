#pragma once

#include "Token.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <typeinfo>

class Scanner
{
    public:
        Scanner(const std::string& source);
        std::vector<Token> scanTokens();

    private:
        std::string source; // Source file in string form.
        std::vector<Token> tokens; // List of all tokens.
        static std::unordered_map<std::string, TokenType> keywords; // Map for all reserved words and their tokens.
        size_t start = 0; // Start of file.
        size_t current = 0; // Current character.
        size_t line = 1; // Number of line we are currently at.

        bool isAtEnd(); // To check if we have reached at the end of source file.
        bool isDigit(char c); // To check if the current char is a digit or not.
        bool isAlpha(char c); // To check if the current char is a alphabet or _.
        bool isAlphaNumeric(char c); // To check if the current chat is alphabet or number.
        bool match(char expected); // To compare the next character with the expected one and skip it.
        char peek(); // To see a character ahead of the current one.
        char peekNext(); // To see two characters ahead of the current one.
        void string(); // To handle string literals token.
        void blockComments(); // Tp handle mutli line comments.
        void number(); // Simlar to string this is used to handle number literals tokens.
        void identifier(); // To handle indentifier(variable) tokens.
        char advance(); // Move to the next character and return the current one.
        void addToken(TokenType type); // Add a single token without literal in tokens vector.
        void addToken(TokenType type, std::variant<std::monostate, std::string, int, double, std::nullptr_t> literal);// Add a single token with literal in tokens vector.
        void scanToken(); // Scan and identify the type of token which is in current index.

};