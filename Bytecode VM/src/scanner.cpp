#include "scanner.h"

Scanner::Scanner(const std::string &src) : src(src)
{
    start = src.c_str();
    current = src.c_str();
    line = 1;
}

Token Scanner::scanToken()
{
    skipWhitespace();
    start = current;

    if(isAtEnd()) return makeToken(TokenType::END_OF_FILE);

    char c = advance();

    if(isDigit(c)) return number();
    if(isAlpha(c)) return identifier();

    switch (c)
    {
        case '(': return makeToken(TokenType::LEFT_PAREN);
        case ')': return makeToken(TokenType::RIGHT_PAREN);
        case '{': return makeToken(TokenType::LEFT_BRACE);
        case '}': return makeToken(TokenType::RIGHT_BRACE);
        case ';': return makeToken(TokenType::SEMICOLON);
        case ',': return makeToken(TokenType::COMMA);
        case '.': return makeToken(TokenType::DOT);
        case '-': return makeToken(TokenType::MINUS);
        case '+': return makeToken(TokenType::PLUS);
        case '/': return makeToken(TokenType::SLASH);
        case '*': return makeToken(TokenType::STAR);

        case '!':
            return makeToken(
                match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
        case '=':
            return makeToken(
                match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
        case '<':
            return makeToken(
                match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
        case '>':
            return makeToken(
                match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
        case '"': return string();
    }

    return errorToken("Unexpected character.");
}

bool Scanner::isAtEnd() const
{
    return *current == '\0';
}

bool Scanner::isDigit(char c) const
{
    return c >= '0' && c <= '9';
}

bool Scanner::isAlpha(char c) const
{
    return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_';
}

Token Scanner::makeToken(TokenType type)
{
    return Token(type, std::string_view(start, current - start), line);
}

Token Scanner::errorToken(const std::string &msg)
{
    return Token(TokenType::ERROR, std::string_view(msg), line);
}

char Scanner::advance()
{
    current++;
    return current[-1];
}

bool Scanner::match(char expected)
{
    if(isAtEnd()) return false;
    if(*current != expected) return false;
    current++;
    return true;
}

char Scanner::peek()
{
    return *current;
}

char Scanner::peekNext()
{
    if(isAtEnd()) return '\0';
    return current[1];
}

std::optional<Token> Scanner::skipWhitespace()
{
    while(true)
    {
        char c = peek();
        switch(c)
        {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                line++;
                advance();
                break;
            case '/' :
                if(peekNext() == '/'){
                    while(peek() != '\n' && !isAtEnd()) advance();
                }
               else if (match('*')) {
                    auto err = blockComment();
                    if (err.has_value()) return err.value(); // propagate error token
                }
                else return std::nullopt;
                break;
            default:
                return std::nullopt;
        }
    }
}

std::optional<Token> Scanner::blockComment()
{
    while (!isAtEnd() && !(peek() == '*' && peekNext() == '/')) {
        if (peek() == '\n') line++;
        advance();
    }

    if (isAtEnd()) {
        return errorToken("Unterminated block comment.");
    }

    advance(); // '*'
    advance(); // '/'
    return std::nullopt; // success, nothing to emit
}

Token Scanner::string()
{
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') line++;
        advance();
    }

    if (isAtEnd()) return errorToken("Unterminated string.");

    // The closing quote.
    advance();
    return makeToken(TokenType::STRING);
}

Token Scanner::number()
{
    while(isDigit(peek())) advance();

    if(peek() == '.' && isDigit(peekNext()))
    {
        advance();

        while(isDigit(peek())) advance();
    }

    return makeToken(TokenType::NUMBER);
}

Token Scanner::identifier()
{
    while(isAlpha(peek()) || isDigit(peek())) advance();
    return makeToken(identifierType());
}

TokenType Scanner::identifierType()
{
    switch(start[0])
    {
    case 'a': return checkKeyword(1, 2, "nd", TokenType::AND);
    case 'c': return checkKeyword(1, 4, "lass", TokenType::CLASS);
    case 'e': return checkKeyword(1, 3, "lse", TokenType::ELSE);
    case 'i': return checkKeyword(1, 1, "f", TokenType::IF);
    case 'n': return checkKeyword(1, 6, "othing", TokenType::NOTHING);
    case 'o': return checkKeyword(1, 1, "r", TokenType::OR);
    case 'p': return checkKeyword(1, 4, "rint", TokenType::PRINT);
    case 'r': return checkKeyword(1, 5, "eturn", TokenType::RETURN);
    case 's': return checkKeyword(1, 4, "uper", TokenType::SUPER);
    case 'l': return checkKeyword(1, 2, "et", TokenType::LET);
    case 'w': return checkKeyword(1, 4, "hile", TokenType::WHILE);
    case 'f': 
        if(current - start > 1)
        {
            switch(start[1])
            {
            case 'a': return checkKeyword(2, 3, "lse", TokenType::FALSE);
            case 'o': return checkKeyword(2, 1, "r", TokenType::FOR);
            case 'u': return checkKeyword(2, 2, "nc", TokenType::FUNC);
            }
        }
        break;
    case 't':
        if(current - start > 1) {
            switch (start[1]) 
            {
            case 'h': return checkKeyword(2, 2, "is", TokenType::THIS);
            case 'r': return checkKeyword(2, 2, "ue", TokenType::TRUE);
            }
        }
        break;
    default:
        break;
    }
    return TokenType::IDENTIFIER;
}

TokenType Scanner::checkKeyword(int start, int length, std::string_view rest, TokenType type)
{
    if(static_cast<int>(current - this -> start) == start + length && 
        std::string_view(this -> start + start, length) == rest) return type;
    
    return TokenType::IDENTIFIER;
}