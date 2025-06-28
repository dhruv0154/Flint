// ------------------------------------------------------------
// Implements the lexical analyzer (scanner) for the Flint language.
// This class takes raw source code as a string and breaks it down into
// a sequence of tokens that represent meaningful language constructs.
//
// Each token corresponds to a language keyword, literal, symbol, or operator.
// The scanner also handles whitespace, comments, and error reporting.
// ------------------------------------------------------------

#include "Scanner\Scanner.h"   // Header for Scanner class and token logic
#include "Scanner\Flint.h"     // Access to error reporting

// ---------------------------------------------------------------------------
// Initialize keyword map: reserved words that have specific token types.
// ---------------------------------------------------------------------------
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
    {"nothing", TokenType::NOTHING},
    {"print", TokenType::PRINT},
    {"return", TokenType::RETURN},
    {"class", TokenType::CLASS},
    {"super", TokenType::SUPER},
    {"this", TokenType::THIS},
    {"var", TokenType::VAR}
};

// ---------------------------------------------------------------------------
// Constructor stores the source text.
// ---------------------------------------------------------------------------
Scanner::Scanner(const std::string& source)
{
    this -> source = source;
}

// ---------------------------------------------------------------------------
// Main scanner loop: scans entire source and returns list of tokens.
// ---------------------------------------------------------------------------
std::vector<Token> Scanner::scanTokens()
{
    while(!isAtEnd())
    {
        start = current; // Start of a new lexeme
        scanToken();     // Scan the next token
    }

    // Add end-of-file token to signify completion
    tokens.push_back(Token(TokenType::END_OF_FILE, "", std::monostate{}, line));
    return tokens;
}

// ---------------------------------------------------------------------------
// Matches one token at a time by character. Handles comments, literals, etc.
// ---------------------------------------------------------------------------
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

        // Handle !, !=, =, ==, <, <=, >, >= operators
        case '!': addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG); break;
        case '=': addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL); break;
        case '<': addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS); break;
        case '>': addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;

        case '/': 
            if(match('/'))
            {
                // Single-line comment: skip to end of line
                while(peek() != '\n' && !isAtEnd()) advance();
            }
            else if(match('*'))
            {
                // Multiline (possibly nested) comment
                blockComments();
            }
            else
            {
                addToken(TokenType::SLASH);
            }
            break;

        case '"': string(); break; // String literal

        // Ignore whitespace
        case ' ': case '\r': case '\t': break;

        case '\n':
            ++line; // Track newlines for error reporting
            break;

        default:
            if(isDigit(c))
            {
                number(); // Number literal
            }
            else if(isAlpha(c))
            {
                identifier(); // Variable name or keyword
            }
            else
            {
                Flint::error(line, "Unexpected character.");
            }
            break;
    }
}

// ---------------------------------------------------------------------------
// Returns true if we've reached the end of the source input.
// ---------------------------------------------------------------------------
bool Scanner::isAtEnd()
{
    return current >= source.length();
}

// ---------------------------------------------------------------------------
// Check if a character is a digit.
// ---------------------------------------------------------------------------
bool Scanner::isDigit(char c)
{
    return c >= '0' && c <= '9';
}

// ---------------------------------------------------------------------------
// Check if a character is alphabetic or underscore.
// ---------------------------------------------------------------------------
bool Scanner::isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

// ---------------------------------------------------------------------------
// Check if a character is alphanumeric or underscore.
// ---------------------------------------------------------------------------
bool Scanner::isAlphaNumeric(char c)
{
    return isAlpha(c) || isDigit(c);
}

// ---------------------------------------------------------------------------
// Consumes the next character if it matches the expected character.
// ---------------------------------------------------------------------------
bool Scanner::match(char expected)
{
    if(isAtEnd()) return false;
    if(source.at(current) != expected) return false;

    current++;
    return true;
}

// ---------------------------------------------------------------------------
// Peek at the current character without advancing.
// ---------------------------------------------------------------------------
char Scanner::peek()
{
    if(isAtEnd()) return '\0';
    return source.at(current);
}

// ---------------------------------------------------------------------------
// Peek one character ahead without advancing.
// ---------------------------------------------------------------------------
char Scanner::peekNext()
{
    if(current + 1 >= source.length()) return '\0';
    return source.at(current + 1);
}

// ---------------------------------------------------------------------------
// Handles string literals, including escape characters.
// ---------------------------------------------------------------------------
void Scanner::string()
{
    std::string value;
    while (!isAtEnd())
    {
        char c = advance();

        if (c == '"') break; // End of string

        if (c == '\\') // Escape sequence
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
            line++;
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

// ---------------------------------------------------------------------------
// Supports nested multiline comments (/* ... */).
// ---------------------------------------------------------------------------
void Scanner::blockComments()
{
    int nestedLevels = 1;

    while (nestedLevels > 0 && !isAtEnd()) 
    {
        if (peek() == '/' && peekNext() == '*') 
        {
            advance(); advance();
            nestedLevels++;
        }
        else if (peek() == '*' && peekNext() == '/') 
        {
            advance(); advance();
            nestedLevels--;
        } 
        else if (peek() == '\n') 
        {
            line++;
            advance();
        }
        else 
        {
            advance();
        }
    }

    if (nestedLevels > 0) 
    {
        Flint::error(line, "Unterminated block comment.");
    }
}

// ---------------------------------------------------------------------------
// Reads and parses number literals, including decimal values.
// ---------------------------------------------------------------------------
void Scanner::number()
{
    while(isDigit(peek())) advance();

    if(peek() == '.' && isDigit(peekNext()))
    {
        advance();
        while(isDigit(peekNext())) advance();
    }

    addToken(TokenType::NUMBER, std::stod(source.substr(start, current - start)));
}

// ---------------------------------------------------------------------------
// Handles identifiers and reserved keywords.
// ---------------------------------------------------------------------------
void Scanner::identifier()
{
    while(isAlphaNumeric(peek())) advance();

    std::string text = source.substr(start, current - start);
    auto it = keywords.find(text);
    TokenType type = (it != keywords.end()) ? it->second : TokenType::IDENTIFIER;
    addToken(type);
}

// ---------------------------------------------------------------------------
// Consumes next character and returns it.
// ---------------------------------------------------------------------------
char Scanner::advance()
{
    current++;
    return source.at(current - 1);
}

// ---------------------------------------------------------------------------
// Overload that adds a token with no literal value.
// ---------------------------------------------------------------------------
void Scanner::addToken(TokenType type)
{
    addToken(type, std::monostate{});
}

// ---------------------------------------------------------------------------
// Adds a new token to the token list.
// ---------------------------------------------------------------------------
void Scanner::addToken(TokenType type, 
    LiteralValue literal)
{
    std::string text = source.substr(start, current - start);
    tokens.push_back(Token(type, text, literal, line));
}
