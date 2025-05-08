#include "Scanner.h"
#include "Flint.h"

std::unordered_map<std::string, TokenType> Scanner::keywords = 
{
    {"and", TokenType::AND},
    {"or", TokenType::OR},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"for", TokenType::FOR},
    {"while", TokenType::WHILE},
    {"fun", TokenType::FUN},
    {"nil", TokenType::NIL},
    {"print", TokenType::PRINT},
    {"return", TokenType::RETURN},
    {"class", TokenType::CLASS},
    {"super", TokenType::SUPER},
    {"this", TokenType::THIS},
    {"var", TokenType::VAR}
};

Scanner::Scanner(const std::string& source)
{
    this -> source = source;
}

std::vector<Token> Scanner::scanTokens()
{
    while(!isAtEnd())
    {
        // We are at beginning of a new lexeme.
        start = current;
        scanToken();
    }

    tokens.push_back(Token(TokenType::END_OF_FILE, "", nullptr, line));
    return tokens;
}

void Scanner::scanToken()
{
    char c = advance();
    switch (c)
    {
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE); break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case ',': addToken(TokenType::COMMA); break;
        case '.': addToken(TokenType::DOT); break;
        case '+': addToken(TokenType::PLUS); break;
        case '-': addToken(TokenType::MINUS); break;
        case ';': addToken(TokenType::SEMICOLON); break;
        case '*': addToken(TokenType::STAR); break;
        case '!': addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG); break;
        case '=': addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL); break;
        case '<': addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS); break;
        case '>': addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;
        case '/': 
            if(match('/'))
            {
                // Skip character until the end of line a comment goes until end of line.

                while(peek() != '\n' && !isAtEnd()) advance();
            }
            else if(match('*'))
            {
                blockComments();
            }
            else
            {
                addToken(TokenType::SLASH);
            }
            break;

        case '"': string(); break;
        case ' ':
        case '\r':
        case '\t':
            // ignore whitespace
            break;
        case '\n':
            ++line;
            break;

        default:
            if(isDigit(c))
            {
                number();
            }
            else if(isAlpha(c))
            {
                identifier();
            }
            else
            {
                Flint::error(line, "Unexpected character.");
            }
            break;
    }
}

bool Scanner::isAtEnd()
{
    return current >= source.length();
}

bool Scanner::isDigit(char c)
{
    return c >= '0' && c <= '9';
}

bool Scanner::isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

bool Scanner::isAlphaNumeric(char c)
{
    return isAlpha(c) || isDigit(c);
}

bool Scanner::match(char expected)
{
    if(isAtEnd()) return false;
    if(source.at(current) != expected) return false;

    current++;
    return true;
}

char Scanner::peek()
{
    if(isAtEnd()) return '\0';

    return source.at(current);
}

char Scanner::peekNext()
{
    if(current + 1 >= source.length()) return '\0';

    return source.at(current + 1);
}

void Scanner::string()
{
    std::string value;
    while (!isAtEnd())
    {
        char c = advance();

        if (c == '"') break;

        if (c == '\\') // start of an escape sequence
        {
            if (isAtEnd()) break;
            char next = advance();
            switch (next)
            {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case 'r': value += '\r'; break;
                case '"': value += '"'; break;
                case '\\': value += '\\'; break;
                default:
                    Flint::error(line, std::string("Invalid escape character: \\") + next);
                    return;
            }
        }
        else if (c == '\n')
        {
            line++; // Count new lines if any (unescaped ones are invalid)
            Flint::error(line, "Unterminated string (unexpected newline).");
            return;
        }
        else
        {
            value += c;
        }
    }

    if (isAtEnd() && source[current - 1] != '"')
    {
        Flint::error(line, "Unterminated string.");
        return;
    }

    addToken(TokenType::STRING, value);
}


void Scanner::blockComments()
{
    int nestedLevels = 1; // Start with one level of nesting (the outermost block comment)

    while (nestedLevels > 0 && !isAtEnd()) 
    {
        // Look ahead for the start of a nested comment (/*)
        if (peek() == '/' && peekNext() == '*') 
        {
            advance(); // Skip over '/'
            advance(); // Skip over '*'
            nestedLevels++; // Increment nesting level
        }
        // Look ahead for the end of a comment (*/)
        else if (peek() == '*' && peekNext() == '/') 
        {
            advance(); // Skip over '*'
            advance(); // Skip over '/'
            nestedLevels--; // Decrement nesting level
        } 
        // Handle newlines inside comments
        else if (peek() == '\n') 
        {
            line++;
            advance();
        }
        else 
        {
            advance(); // Skip other characters in the comment
        }
    }

    if (nestedLevels > 0) 
    {
        Flint::error(line, "Unterminated block comment.");
    }
}

void Scanner::number()
{
    while(isDigit(peek())) advance();

    // Look for a . for decimal numbers.

    if(peek() == '.' && isDigit(peekNext()))
    {
        // Get past the . using advance.
        advance();

        // Keep skiping characters until they are digits
        while(isDigit(peekNext())) advance();
    }

    addToken(TokenType::NUMBER, std::stod(source.substr(start, current - start)));
}

void Scanner::identifier()
{
    while(isAlphaNumeric(peek())) advance();

    std::string text = source.substr(start, current - start);
    auto it = keywords.find(text);
    TokenType type = (it != keywords.end()) ? it->second : TokenType::IDENTIFIER;
    addToken(type);
}

char Scanner::advance()
{
    current++;
    return source.at(current - 1);
}

void Scanner::addToken(TokenType type)
{
    addToken(type, nullptr);
}

void Scanner::addToken(TokenType type, std::any literal)
{
    std::string text = source.substr(start, current - start);
    tokens.push_back(Token(type, text, literal, line));
}