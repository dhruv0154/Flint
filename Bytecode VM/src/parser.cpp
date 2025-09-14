#include <iostream>
#include <charconv>
#include "compiler.h"
#include "C:\Users\HP\OneDrive\Flint\Flint\Bytecode VM\include\parser.h"


void Parser::advance()
{
    previous = current;

    while(true)
    {
        current = scanner.scanToken();
        if(current.type != TokenType::ERROR) break;

        errorAtCurrent(current.lexeme);
    }
    
}

void Parser::expression()
{
    parsePrecedence(Precedence::PREC_ASSIGNMENT);
}

void Parser::number(Parser& p)
{
    double value;
    auto result = std::from_chars(p.previous.lexeme.data(), 
        p.previous.lexeme.data() + p.previous.lexeme.size(), value);
    
    if(result.ec == std::errc())
    {
        p.compiler -> emitConstant(value);
    }
}

void Parser::grouping(Parser& p)
{
    p.expression();
    p.consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
}

void Parser::unary(Parser& p)
{
    TokenType op = p.previous.type;

    p.parsePrecedence(Precedence::PREC_UNARY);

    switch (op)
    {
    case TokenType::MINUS:
        p.compiler -> emitByte(static_cast<uint8_t>(OpCode::OP_NEGATE));
        break;
    
    default:
        return;
    }
}

void Parser::binary(Parser& p)
{
    TokenType op = p.previous.type;
    const ParseRule& rule = p.getRule(op);
    p.parsePrecedence(static_cast<Precedence>(rule.precedence + 1));

    switch (op)
    {
    case TokenType::PLUS: p.compiler -> emitByte(static_cast<int>(OpCode::OP_ADD)); break;
    case TokenType::MINUS: p.compiler -> emitByte(static_cast<int>(OpCode::OP_SUBTRACT)); break;
    case TokenType::STAR: p.compiler -> emitByte(static_cast<int>(OpCode::OP_MULTIPLY)); break;
    case TokenType::SLASH: p.compiler -> emitByte(static_cast<int>(OpCode::OP_DIVIDE)); break;
    default: return;
    }
}

const ParseRule& Parser::getRule(TokenType type)
{
    return rules.at(static_cast<size_t>(type));
}

void Parser::parsePrecedence(Precedence precedence)
{
    advance();
    ParseFn prefixRule = getRule(previous.type).prefix;

    if(prefixRule == nullptr)
    {
        error("Expect expression.");
        return;
    }

    prefixRule(*this);

    while(precedence <= getRule(current.type).precedence)
    {
        advance();
        ParseFn infixRule = getRule(previous.type).infix;
        infixRule(*this);
    }
}

void Parser::consume(TokenType type, const std::string_view &msg)
{
    if(current.type == type) 
    {
        advance();
        return;
    }
    errorAtCurrent(msg);
}

void Parser::errorAtCurrent(const std::string_view &msg)
{
    errorAt(&current, msg);
}

void Parser::error(const std::string_view &msg)
{
    errorAt(&previous, msg);
}

void Parser::errorAt(Token *token, const std::string_view &msg)
{
    if(panicMode) return;
    panicMode = true;
    std::cerr << "[line " << token -> line << "]";

    if(token->type == TokenType::END_OF_FILE)
    {
        std::cerr << " at end";
    } 
    else if(token -> type == TokenType::ERROR)
    {
        // do nothing
    }
    else
    {
        std::cerr << " at " << token -> lexeme;
    }

    std::cerr << ": " << msg << "\n";
    hadError = true;
}