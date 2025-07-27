// ------------------------------------------------------------
// Implements the lexical analyzer (scanner) for the Flint language.
// This class takes raw source code as a string and breaks it down into
// a sequence of tokens that represent meaningful language constructs.
//
// Each token corresponds to a language keyword, literal, symbol, or operator.
// The scanner also handles whitespace, comments, and error reporting.
// ------------------------------------------------------------

#include "Flint/Scanner/Scanner.h"
#include "Flint/Flint.h"

// ---------------------------------------------------------------------------
// Static map of reserved keywords mapped to their TokenTypes.
// If an identifier matches one of these, it’s emitted as that keyword token.
// ---------------------------------------------------------------------------
std::unordered_map<std::string, TokenType> Scanner::keywords = 
{
    {"and",      TokenType::AND},
    {"or",       TokenType::OR},
    {"if",       TokenType::IF},
    {"else",     TokenType::ELSE},
    {"true",     TokenType::TRUE},
    {"false",    TokenType::FALSE},
    {"for",      TokenType::FOR},
    {"while",    TokenType::WHILE},
    {"break",    TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"func",     TokenType::FUNC},
    {"nothing",  TokenType::NOTHING},
    {"return",   TokenType::RETURN},
    {"class",    TokenType::CLASS},
    {"super",    TokenType::SUPER},
    {"this",     TokenType::THIS},
    {"let",      TokenType::LET}
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
// Adds an EOF token at the end to signal end-of-input.
// ---------------------------------------------------------------------------
std::vector<Token> Scanner::scanTokens()
{
    // Continue until we've consumed every character
    while (!isAtEnd())
    {
        start = current;     // mark beginning of next lexeme
        scanToken();         // scan one token
    }

    // Append end-of-file token so parser knows when to stop
    tokens.push_back(Token(TokenType::END_OF_FILE, "", std::monostate{}, line));
    return tokens;
}

// ---------------------------------------------------------------------------
// Reads and processes a single token based on the next character.
// Handles operators, literals, comments, whitespace, and errors.
// ---------------------------------------------------------------------------
void Scanner::scanToken()
{
    char c = advance();  // consume next char
    switch (c)
    {
        // Single-character tokens
        case '(': addToken(TokenType::LEFT_PAREN);  break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE);  break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case ',': addToken(TokenType::COMMA);       break;
        case '.': addToken(TokenType::DOT);         break;
        case '+': addToken(TokenType::PLUS);        break;
        case '-': addToken(TokenType::MINUS);       break;
        case ';': addToken(TokenType::SEMICOLON);   break;
        case '*': addToken(TokenType::STAR);        break;
        case '%': addToken(TokenType::MODULO);      break;
        case ':': addToken(TokenType::COLON);       break;
        case '?': addToken(TokenType::QUESTION_MARK); break;

        // Handle '&&' and '&' with custom error messages
        case '&':
            if (match('&')) {
                // discourage C-style logical ops
                Flint::error(current - 2, "Use 'and' instead of '&&'");
            } else {
                Flint::error(current - 1, "Unexpected character '&'");
            }
            break;

        // Handle '||' and '|'
        case '|':
            if (match('|')) {
                Flint::error(current - 2, "Use 'or' instead of '||'");
            } else {
                Flint::error(current - 1, "Unexpected character '|'");
            }
            break;

        // Two‑character operators: !=, ==, <=, >=
        case '!':
            addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
            break;
        case '=':
            addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
            break;
        case '<':
            addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
            break;
        case '>':
            addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
            break;

        // Slash could start comment or be division operator
        case '/':
            if (match('/')) {
                // single-line comment: skip until newline
                while (peek() != '\n' && !isAtEnd()) advance();
            }
            else if (match('*')) {
                // multi-line (possibly nested) comment
                blockComments();
            }
            else {
                // plain slash token
                addToken(TokenType::SLASH);
            }
            break;

        // Start of string literal
        case '"':
            string();
            break;

        // Whitespace is ignored
        case ' ':
        case '\r':
        case '\t':
            break;

        // Newline: increment line counter for error reporting
        case '\n':
            ++line;
            break;

        default:
            // Digits start numeric literal
            if (isDigit(c)) {
                number();
            }
            // Letters or underscore start identifier/keyword
            else if (isAlpha(c)) {
                identifier();
            }
            // Anything else is unrecognized
            else {
                Flint::error(line, "Unexpected character.");
            }
            break;
    }
}

// ---------------------------------------------------------------------------
// Returns true if we've consumed all characters of the source.
// ---------------------------------------------------------------------------
bool Scanner::isAtEnd() const
{
    return current >= source.length();
}

// ---------------------------------------------------------------------------
// Character classification helpers.
// ---------------------------------------------------------------------------
bool Scanner::isDigit(char c) const
{
    return c >= '0' && c <= '9';
}

bool Scanner::isAlpha(char c) const
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

bool Scanner::isAlphaNumeric(char c) const
{
    return isAlpha(c) || isDigit(c);
}

// ---------------------------------------------------------------------------
// Conditionally consumes the next character if it matches `expected`.
// Returns true and advances if so; otherwise returns false.
// ---------------------------------------------------------------------------
bool Scanner::match(char expected)
{
    if (isAtEnd()) return false;
    if (source.at(current) != expected) return false;

    current++;
    return true;
}

// ---------------------------------------------------------------------------
// Peeks at the current character without consuming it.
// ---------------------------------------------------------------------------
char Scanner::peek() const
{
    return isAtEnd() ? '\0' : source.at(current);
}

// ---------------------------------------------------------------------------
// Peeks one character ahead without consuming it.
// ---------------------------------------------------------------------------
char Scanner::peekNext() const
{
    return (current + 1 >= source.length()) ? '\0' : source.at(current + 1);
}

// ---------------------------------------------------------------------------
// Parses and stores a string literal, handling escape sequences and errors.
// Stops at the closing '"' or reports unterminated-string.
// ---------------------------------------------------------------------------
void Scanner::string()
{
    std::string value;

    while (!isAtEnd())
    {
        char c = advance();

        if (c == '"') break;  // closing quote found

        if (c == '\\') {
            // handle escape
            if (isAtEnd()) break;
            char next = advance();
            switch (next) {
                case 'n':  value += '\n'; break;
                case 't':  value += '\t'; break;
                case 'r':  value += '\r'; break;
                case '"':  value += '"';  break;
                case '\\': value += '\\'; break;
                default:
                    Flint::error(line, std::string("Invalid escape: \\") + next);
                    return;
            }
        }
        else if (c == '\n') {
            // newline inside string is illegal
            ++line;
            Flint::error(line, "Unterminated string (newline encountered).");
            return;
        }
        else {
            value += c;  // normal character
        }
    }

    // if we exited without closing quote, report error
    if (isAtEnd() && source[current - 1] != '"') {
        Flint::error(line, "Unterminated string.");
        return;
    }

    // emit the STRING token with its extracted value
    addToken(TokenType::STRING, value);
}

// ---------------------------------------------------------------------------
// Handles nested multi-line comments /* ... */ at any depth.
// Increments `nestedLevels` for each /* and decrements for each */.
// Tracks line numbers inside comments.
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
        else {
            if (peek() == '\n') ++line;
            advance();
        }
    }

    if (nestedLevels > 0) {
        Flint::error(line, "Unterminated block comment.");
    }
}

// ---------------------------------------------------------------------------
// Parses numeric literals, including optional fractional part.
// Consumes all digits, then '.' and more digits if present.
// ---------------------------------------------------------------------------
void Scanner::number()
{
    // consume leading digits
    while (isDigit(peek())) advance();

    // optional fractional part
    if (peek() == '.' && isDigit(peekNext())) {
        advance();  // consume '.'
        while (isDigit(peek())) advance();
    }

    // convert text to double
    std::string text = source.substr(start, current - start);
    addToken(TokenType::NUMBER, std::stod(text));
}

// ---------------------------------------------------------------------------
// Parses an identifier or checks if it's a reserved keyword.
// Consumes alphanumeric characters and underscores.
// ---------------------------------------------------------------------------
void Scanner::identifier()
{
    while (isAlphaNumeric(peek())) advance();

    std::string text = source.substr(start, current - start);
    auto it = keywords.find(text);

    // if it's a keyword, use its type; otherwise IDENTIFIER
    TokenType type = (it != keywords.end())
                   ? it->second
                   : TokenType::IDENTIFIER;

    addToken(type);
}

// ---------------------------------------------------------------------------
// Advances the cursor by one and returns the consumed character.
// ---------------------------------------------------------------------------
char Scanner::advance()
{
    current++;
    return source.at(current - 1);
}

// ---------------------------------------------------------------------------
// Adds a token with no literal value (e.g., punctuation, operators).
// ---------------------------------------------------------------------------
void Scanner::addToken(TokenType type)
{
    addToken(type, std::monostate{});
}

// ---------------------------------------------------------------------------
// Adds a token with an associated literal value (e.g., number, string).
// Extracts the lexeme text and stores line number.
// ---------------------------------------------------------------------------
void Scanner::addToken(TokenType type, LiteralValue literal)
{
    std::string lexeme = source.substr(start, current - start);
    tokens.push_back(Token(type, lexeme, literal, line));
}