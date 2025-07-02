#include "Parser\Parser.h"
#include "Flint\Flint.h"

std::shared_ptr<ExpressionNode> Parser::expression()
{
    return conditional();
}

std::shared_ptr<ExpressionNode> Parser::conditional()
{
    std::shared_ptr<ExpressionNode> expr = comma();

    // Ternary is right-associative: use if, not while
    if (match({ TokenType::QUESTION_MARK }))
    {
        std::shared_ptr<ExpressionNode> thenBranch = conditional();
        consume(TokenType::COLON, "Expected ':' after then branch of ternary operator.");
        std::shared_ptr<ExpressionNode> elseBranch = conditional(); // Recursive
        expr = makeExpr<Conditional>(expr, thenBranch, elseBranch);
    }

    return expr;
}


std::shared_ptr<ExpressionNode> Parser::comma()
{
    // Error production: leading comma
    if (match({ TokenType::COMMA })) {
        Token op = previous();
        error(op, "Missing left-hand operand before ','.");
        // Recover by parsing RHS
        return equality();
    }

    // Normal comma parsing
    auto expr = equality();
    while (match({ TokenType::COMMA })) {
        Token op = previous();
        auto right = equality();
        expr = makeExpr<Binary>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<ExpressionNode> Parser::equality()
{
    // Error production: leading == or !=
    if (match({ TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL })) {
        Token op = previous();
        error(op, "Missing left-hand operand before '" + op.lexeme + "'.");
        return comparison();
    }

    // Normal equality parsing
    auto expr = comparison();
    while (match({ TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL })) {
        Token op = previous();
        auto right = comparison();
        expr = makeExpr<Binary>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<ExpressionNode> Parser::comparison()
{
    // Error production: leading <, <=, >, >=
    if (match({ 
            TokenType::LESS, TokenType::LESS_EQUAL, 
            TokenType::GREATER, TokenType::GREATER_EQUAL 
        })) 
    {
        Token op = previous();
        error(op, "Missing left-hand operand before '" + op.lexeme + "'.");
        return term();
    }

    // Normal comparison parsing
    auto expr = term();
    while (match({
            TokenType::LESS, TokenType::LESS_EQUAL,
            TokenType::GREATER, TokenType::GREATER_EQUAL
        }))
    {
        Token op = previous();
        auto right = term();
        expr = makeExpr<Binary>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<ExpressionNode> Parser::term()
{
    // Error production: leading + or -
    if (match({ TokenType::PLUS, TokenType::MINUS })) {
        Token op = previous();
        error(op, "Missing left-hand operand before '" + op.lexeme + "'.");
        return factor();
    }

    // Normal term parsing
    auto expr = factor();
    while (match({ TokenType::PLUS, TokenType::MINUS })) {
        Token op = previous();
        auto right = factor();
        expr = makeExpr<Binary>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<ExpressionNode> Parser::factor()
{
    // Error production: leading * or /
    if (match({ TokenType::STAR, TokenType::SLASH })) {
        Token op = previous();
        error(op, "Missing left-hand operand before '" + op.lexeme + "'.");
        return unary();
    }

    // Normal factor parsing
    auto expr = unary();
    while (match({ TokenType::STAR, TokenType::SLASH })) {
        Token op = previous();
        auto right = unary();
        expr = makeExpr<Binary>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<ExpressionNode> Parser::unary()
{
    if(match({ TokenType::BANG, TokenType::MINUS }))
    {
        Token op = previous();
        std::shared_ptr<ExpressionNode> right = unary();
        return makeExpr<Unary>(op, right);
    }

    return primary();
}

std::shared_ptr<ExpressionNode> Parser::primary()
{
    if(match({ TokenType::FALSE }))
        return makeExpr<Literal>(false);

    if(match({ TokenType::TRUE }))
        return makeExpr<Literal>(true);

    if(match({ TokenType::NOTHING }))
        return makeExpr<Literal>(std::monostate{});

    if(match({ TokenType::NUMBER, TokenType::STRING }))
        return makeExpr<Literal>(previous().literal);

    if(match({ TokenType::LEFT_PAREN }))
    {
        std::shared_ptr<ExpressionNode> expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expected a ')'.");
        return makeExpr<Grouping>(expr);
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
        case TokenType::FUNC:
        case TokenType::LET:
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

template<typename T, typename... Args>
std::shared_ptr<ExpressionNode> Parser::makeExpr(Args&&... args) 
{
    return std::make_shared<ExpressionNode>(T{std::forward<Args>(args)...});
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