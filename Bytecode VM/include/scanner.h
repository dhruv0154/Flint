#pragma once

#include <string>
#include <optional>
#include "Token.h"

class Scanner
{
private:
    std::string src;
    const char* start;
    const char* current;
    size_t line;
    Token makeToken(TokenType type);
    Token errorToken(const std::string &msg);
    bool isAtEnd() const;
    bool isDigit(char c) const;
    bool isAlpha(char c) const;
    bool match(char expected);
    char advance();
    char peek();
    char peekNext();
    std::optional<Token> skipWhitespace();
    std::optional<Token> blockComment();
    Token string();
    Token number();
    Token identifier();
    TokenType identifierType();
    TokenType checkKeyword(int start, int length, std::string_view rest, TokenType type);
public:
    Token scanToken();
    Scanner(const std::string &src);
    Scanner() = default;
    ~Scanner() = default;

    friend class Compiler;
};