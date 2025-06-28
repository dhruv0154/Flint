#include "C:\Flint\include\Parser\Parser.h"
#include "C:\Flint\include\Scanner\Flint.h"

std::shared_ptr<ExpressionNode> Parser::expression()
{
    return equality();
}

std::shared_ptr<ExpressionNode> Parser::equality()
{
    std::shared_ptr<ExpressionNode> expr = comparison();

    while(match({ TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL }))
    {
        Token op = previous();
        std::shared_ptr<ExpressionNode> right = comparison();
        expr = std::make_shared<ExpressionNode>(Binary{expr, op, right});
    }

    return expr;
}

std::shared_ptr<ExpressionNode> Parser::comparison()
{
    std::shared_ptr<ExpressionNode> expr = term();

    while(match({ TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL }))
    {
        Token op = previous();
        std::shared_ptr<ExpressionNode> right = term();
        expr = std::make_shared<ExpressionNode>(Binary{expr, op, right});
    }

    return expr;
}

std::shared_ptr<ExpressionNode> Parser::term()
{
    std::shared_ptr<ExpressionNode> expr = factor();

    while(match({ TokenType::PLUS, TokenType::MINUS }))
    {
        Token op = previous();
        std::shared_ptr<ExpressionNode> right = factor();
        expr = std::make_shared<ExpressionNode>(Binary{expr, op, right});
    }

    return expr;
}

std::shared_ptr<ExpressionNode> Parser::factor()
{
    std::shared_ptr<ExpressionNode> expr = unary();

    while(match({ TokenType::SLASH, TokenType::STAR }))
    {
        Token op = previous();
        std::shared_ptr<ExpressionNode> right = unary();
        expr = std::make_shared<ExpressionNode>(Binary{expr, op, right});
    }

    return expr;
}

std::shared_ptr<ExpressionNode> Parser::unary()
{
    if(match({ TokenType::BANG, TokenType::MINUS }))
    {
        Token op = previous();
        std::shared_ptr<ExpressionNode> right = unary();
        return std::make_shared<ExpressionNode>(Unary{op, right});
    }

    return primary();
}

std::shared_ptr<ExpressionNode> Parser::primary()
{
    if(match({ TokenType::FALSE }))
        return std::make_shared<ExpressionNode>(Literal{false});

    if(match({ TokenType::TRUE }))
        return std::make_shared<ExpressionNode>(Literal{true});;

    if(match({ TokenType::NOTHING }))
        return std::make_shared<ExpressionNode>(Literal{std::monostate{}});

    if(match({ TokenType::NUMBER, TokenType::STRING }))
        return std::make_shared<ExpressionNode>(Literal{previous().literal});

    if(match({ TokenType::LEFT_PAREN }))
    {
        std::shared_ptr<ExpressionNode> expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expected a ')'.");
        return std::make_shared<ExpressionNode>(Grouping{expr});
    }

    throw error(peek(), "Expected an expression.");
}

std::shared_ptr<ExpressionNode> Parser::parse()
{
   try
   {
        return expression();
   }
   catch(ParseError error)
   {
        return nullptr;
   }
}

bool Parser::match(const std::vector<TokenType>& types)
{
    for(TokenType type : types)
    {
        if(check(type))
        {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::check(TokenType type)
{
    if(isAtEnd()) return false;
    return peek().type == type;
}

Token Parser::advance()
{
    if(!isAtEnd()) current++;
    return previous();
}

Token Parser::consume(TokenType type, std::string message)
{
    if(check(type)) return advance();

    throw error(peek(), message);
}

void Parser::synchronize()
{
    advance();

    while(!isAtEnd())
    {
        if(previous().type == TokenType::SEMICOLON) return;

        switch (previous().type)
        {
        case TokenType::CLASS:
        case TokenType::FUN:
        case TokenType::VAR:
        case TokenType::FOR:
        case TokenType::IF:
        case TokenType::WHILE:
        case TokenType::PRINT:
        case TokenType::RETURN:
            return;
        }
        advance();
    }
}

Parser::ParseError Parser::error(Token token, std::string message)
{
    Flint::error(token, message);
    return ParseError(message);
}

bool Parser::isAtEnd()
{
    return peek().type == TokenType::END_OF_FILE;
}

Token Parser::peek()
{
    return tokens.at(current);
}

Token Parser::previous()
{
    return tokens.at(current - 1);
}