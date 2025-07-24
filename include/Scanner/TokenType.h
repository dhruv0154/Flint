#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  TokenType.h – Enumerates All Tokens for Flint
// ─────────────────────────────────────────────────────────────────────────────
//  Defines every token category recognized by the Scanner and used by the Parser.
//  Token types are grouped by their syntactic role:
//    1. Single-character symbols
//    2. One- or two-character operators
//    3. Literals (identifiers, strings, numbers)
//    4. Keywords (reserved words)
//    5. End-of-file marker
// ─────────────────────────────────────────────────────────────────────────────

enum class TokenType {
    // ─────────────────────────────────────────────────────────────
    // 1. Single-character symbols
    //    Represent punctuation and delimiters in the language
    // ─────────────────────────────────────────────────────────────
    LEFT_PAREN,     // (  Open grouping or function call
    RIGHT_PAREN,    // )  Close grouping or function call
    LEFT_BRACE,     // {  Begin block
    RIGHT_BRACE,    // }  End block
    COMMA,          // ,  Separator in lists or parameters
    DOT,            // .  Property access or method invocation
    MINUS,          // -  Subtraction or negation
    PLUS,           // +  Addition or unary plus
    MODULO,         // %  Remainder operator
    COLON,          // :  Used in ternary and other constructs
    QUESTION_MARK,  // ?  Begins ternary conditional
    SEMICOLON,      // ;  Statement terminator
    SLASH,          // /  Division or comment start
    STAR,           // *  Multiplication

    // ─────────────────────────────────────────────────────────────
    // 2. One- or two-character operators
    //    Cover comparisons and logical negation/equality
    // ─────────────────────────────────────────────────────────────
    BANG,           // !  Logical NOT
    BANG_EQUAL,     // != Not equal comparison
    EQUAL,          // =  Assignment or equality in LHS check
    EQUAL_EQUAL,    // == Equality comparison
    GREATER,        // >  Greater-than comparison
    GREATER_EQUAL,  // >= Greater-than-or-equal comparison
    LESS,           // <  Less-than comparison
    LESS_EQUAL,     // <= Less-than-or-equal comparison

    // ─────────────────────────────────────────────────────────────
    // 3. Literals
    //    Tokens representing values written directly in code
    // ─────────────────────────────────────────────────────────────
    IDENTIFIER,     // Names for variables, functions, classes
    STRING,         // Quoted string literal
    NUMBER,         // Numeric literal (integer or floating-point)

    // ─────────────────────────────────────────────────────────────
    // 4. Keywords
    //    Reserved words with special meaning in the language
    // ─────────────────────────────────────────────────────────────
    AND,            // and Logical conjunction
    CLASS,          // class Class declaration
    ELSE,           // else Conditional alternative
    FALSE,          // false Boolean literal
    FUNC,           // func Function declaration
    FOR,            // for Loop construct
    IF,             // if Conditional construct
    NOTHING,        // nothing Null/nil equivalent
    OR,             // or Logical disjunction
    RETURN,         // return Function return
    SUPER,          // super Access to superclass methods
    THIS,           // this Reference to current instance
    TRUE,           // true Boolean literal
    LET,            // let Variable declaration
    WHILE,          // while Loop construct
    BREAK,          // break Exit loop early
    CONTINUE,       // continue Skip to next loop iteration

    // ─────────────────────────────────────────────────────────────
    // 5. End-of-File
    //    Marks end of token stream
    // ─────────────────────────────────────────────────────────────
    END_OF_FILE     // EOF Token
};
