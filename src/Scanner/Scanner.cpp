// ------------------------------------------------------------
// Implements the lexical analyzer (scanner) for the Flint language.
// This class takes raw source code as a string and breaks it down into
// a sequence of tokens that represent meaningful language constructs.
//
// Each token corresponds to a language keyword, literal, symbol, or operator.
// The scanner also handles whitespace, comments, and error reporting.
// ------------------------------------------------------------

#include "Scanner\Scanner.h"
#include "Flint\Flint.h"

// ---------------------------------------------------------------------------
// Static map of reserved keywords mapped to their TokenTypes.
// If an identifier matches one of these, it’s parsed as a keyword.
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
    {"func", TokenType::FUNC},
    {"nothing", TokenType::NOTHING},
    {"print", TokenType::PRINT},
    {"return", TokenType::RETURN},
    {"class", TokenType::CLASS},
    {"super", TokenType::SUPER},
    {"this", TokenType::THIS},
    {"let", TokenType::LET}
};

// ---------------------------------------------------------------------------
// Scanner constructor stores the source code string.
// ---------------------------------------------------------------------------
Scanner::Scanner(const std::string& source)
{
    this->source = source;
}

// ---------------------------------------------------------------------------
// Main scanner loop: tokenizes the entire source code into tokens.
// Adds an EOF token at the end.
// ---------------------------------------------------------------------------
std::vector<Token> Scanner::scanTokens()
{
    while (!isAtEnd())
    {
        start = current;
        scanToken();
    }

    tokens.push_back(Token(TokenType::END_OF_FILE, "", std::monostate{}, line));
    return tokens;
}

// ---------------------------------------------------------------------------
// Reads and processes a single token based on the next character.
// Handles operators, literals, comments, whitespace, and errors.
// ---------------------------------------------------------------------------
void Scanner::scanToken()
{
    char c = advance();
    switch (c)
    {
        // Single-character tokens
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
        case '%': addToken(TokenType::MODULO); break;
        case ':': addToken(TokenType::COLON); break;
        case '?': addToken(TokenType::QUESTION_MARK); break;

        // Operators with possible two-character forms
        case '!': addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG); break;
        case '=': addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL); break;
        case '<': addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS); break;
        case '>': addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;

        case '/': 
            if (match('/')) {
                // Single-line comment: ignore until newline
                while (peek() != '\n' && !isAtEnd()) advance();
            }
            else if (match('*')) {
                // Multi-line or nested comment
                blockComments();
            }
            else {
                addToken(TokenType::SLASH);
            }
            break;

        case '"':
            string(); // String literal
            break;

        // Whitespace (ignored)
        case ' ':
        case '\r':
        case '\t':
            break;

        // Newline: track line number
        case '\n':
            ++line;
            break;

        default:
            // Number literals
            if (isDigit(c)) {
                number();
            }
            // Identifiers or keywords
            else if (isAlpha(c)) {
                identifier();
            }
            // Unknown character
            else {
                Flint::error(line, "Unexpected character.");
            }
            break;
    }
}

// ---------------------------------------------------------------------------
// Returns true if we're at the end of the input string.
// ---------------------------------------------------------------------------
bool Scanner::isAtEnd()
{
    return current >= source.length();
}

// ---------------------------------------------------------------------------
// Character classification helpers
// ---------------------------------------------------------------------------
bool Scanner::isDigit(char c)
{
    return c >= '0' && c <= '9';
}

bool Scanner::isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

bool Scanner::isAlphaNumeric(char c)
{
    return isAlpha(c) || isDigit(c);
}

// ---------------------------------------------------------------------------
// Matches a character only if it matches the expected one.
// Advances the cursor if matched.
// ---------------------------------------------------------------------------
bool Scanner::match(char expected)
{
    if (isAtEnd()) return false;
    if (source.at(current) != expected) return false;

    current++;
    return true;
}

// ---------------------------------------------------------------------------
// Returns the current character without consuming it.
// ---------------------------------------------------------------------------
char Scanner::peek()
{
    if (isAtEnd()) return '\0';
    return source.at(current);
}

// ---------------------------------------------------------------------------
// Returns the next character (lookahead) without advancing.
// ---------------------------------------------------------------------------
char Scanner::peekNext()
{
    if (current + 1 >= source.length()) return '\0';
    return source.at(current + 1);
}

// ---------------------------------------------------------------------------
// Parses and stores a string literal, handling escape sequences.
// ---------------------------------------------------------------------------
void Scanner::string()
{
    std::string value;

    while (!isAtEnd())
    {
        char c = advance();

        if (c == '"') break; // Closing quote

        if (c == '\\') {
            if (isAtEnd()) break;
            char next = advance();

            switch (next) {
                case 'n':  value += '\n'; break;
                case 't':  value += '\t'; break;
                case 'r':  value += '\r'; break;
                case '"':  value += '"'; break;
                case '\\': value += '\\'; break;
                default:
                    Flint::error(line, std::string("Invalid escape character: \\") + next);
                    return;
            }
        }
        else if (c == '\n') {
            line++;
            Flint::error(line, "Unterminated string (unexpected newline).");
            return;
        }
        else {
            value += c;
        }
    }

    if (isAtEnd() && source[current - 1] != '"') {
        Flint::error(line, "Unterminated string.");
        return;
    }

    addToken(TokenType::STRING, value);
}

// ---------------------------------------------------------------------------
// Handles nested multi-line comments: /* ... */ (including nested ones)
// ---------------------------------------------------------------------------
void Scanner::blockComments()
{
    int nestedLevels = 1;

    while (nestedLevels > 0 && !isAtEnd()) 
    {
        if (peek() == '/' && peekNext() == '*') {
            advance(); advance();
            nestedLevels++;
        }
        else if (peek() == '*' && peekNext() == '/') {
            advance(); advance();
            nestedLevels--;
        }
        else if (peek() == '\n') {
            line++;
            advance();
        }
        else {
            advance();
        }
    }

    if (nestedLevels > 0) {
        Flint::error(line, "Unterminated block comment.");
    }
}

// ---------------------------------------------------------------------------
// Parses numeric literals: integers and floating-point values.
// ---------------------------------------------------------------------------
void Scanner::number()
{
    while (isDigit(peek())) advance();

    if (peek() == '.' && isDigit(peekNext())) {
        advance(); // consume '.'
        while (isDigit(peek())) advance();
    }

    std::string numberText = source.substr(start, current - start);
    addToken(TokenType::NUMBER, std::stod(numberText));
}

// ---------------------------------------------------------------------------
// Parses an identifier (or keyword if matched in the keyword map).
// ---------------------------------------------------------------------------
void Scanner::identifier()
{
    while (isAlphaNumeric(peek())) advance();

    std::string text = source.substr(start, current - start);
    auto it = keywords.find(text);
    TokenType type = (it != keywords.end()) ? it->second : TokenType::IDENTIFIER;

    addToken(type);
}

// ---------------------------------------------------------------------------
// Advances by one character and returns it.
// ---------------------------------------------------------------------------
char Scanner::advance()
{
    current++;
    return source.at(current - 1);
}

// ---------------------------------------------------------------------------
// Adds a token without a literal value (e.g., operators).
// ---------------------------------------------------------------------------
void Scanner::addToken(TokenType type)
{
    addToken(type, std::monostate{});
}

// ---------------------------------------------------------------------------
// Adds a token with optional literal value.
// ---------------------------------------------------------------------------
void Scanner::addToken(TokenType type, LiteralValue literal)
{
    std::string text = source.substr(start, current - start);
    tokens.push_back(Token(type, text, literal, line));
}