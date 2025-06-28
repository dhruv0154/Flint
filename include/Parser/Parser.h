#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include "C:\Flint\include\Scanner\TokenType.h"
#include "C:\Flint\include\Scanner\Token.h"
#include "C:\Flint\include\Scanner\generated\Expr.h"

class Parser
{
public:
    class ParseError : public std::runtime_error
    {
    public:
        ParseError(const std::string& message) : std::runtime_error(message) {}; 
    };

private:
    std::vector<Token> tokens;
    int current = 0;

    
    std::shared_ptr<ExpressionNode> expression();
    std::shared_ptr<ExpressionNode> equality();
    std::shared_ptr<ExpressionNode> comparison();
    std::shared_ptr<ExpressionNode> term();
    std::shared_ptr<ExpressionNode> factor();
    std::shared_ptr<ExpressionNode> unary();
    std::shared_ptr<ExpressionNode> primary();
    bool match(const std::vector<TokenType>& types);
    bool check(TokenType type);
    bool isAtEnd();
    Token advance();
    Token consume(TokenType type, std::string message);
    Token peek();
    Token previous();
    void synchronize();
    [[nodiscard]] Parser::ParseError error(Token token, std::string message);

public:
    std::shared_ptr<ExpressionNode> parse();
    Parser(std::vector<Token> tokens) : tokens(tokens) {};
    ~Parser() = default;
};