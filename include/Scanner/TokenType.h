#pragma once

// All possible token types recognized by the scanner
enum class TokenType 
{
    // Single-character tokens (used in expressions and blocks)
    LEFT_PAREN,    // (
    RIGHT_PAREN,   // )
    LEFT_BRACE,    // {
    RIGHT_BRACE,   // }
    COMMA,         // ,
    DOT,           // .
    MINUS,         // -
    PLUS,          // +
    COLON,         // :
    QUESTION_MARK, // ?
    SEMICOLON,     // ;
    SLASH,         // /
    STAR,          // *

    // One or two-character operators
    BANG,          // !
    BANG_EQUAL,    // !=
    EQUAL,         // =
    EQUAL_EQUAL,   // ==
    GREATER,       // >
    GREATER_EQUAL, // >=
    LESS,          // <
    LESS_EQUAL,    // <=

    // Literals
    IDENTIFIER,    // Variable names, function names, etc.
    STRING,        // String literals (e.g. "hello")
    NUMBER,        // Numeric literals (int or float)

    // Keywords (reserved words with special meaning)
    AND,
    CLASS,
    ELSE,
    FALSE,
    FUNC,
    FOR,
    IF,
    NOTHING,
    OR,
    PRINT,
    RETURN,
    SUPER,
    THIS,
    TRUE,
    LET,
    WHILE,

    // Special token to indicate the end of input
    END_OF_FILE
};