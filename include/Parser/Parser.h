#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  Parser Class
// ─────────────────────────────────────────────────────────────────────────────
//  Converts a flat stream of tokens (from the scanner) into a structured
//  Abstract Syntax Tree (AST) composed of `ExpressionNode`s and `Statement`s.
//
//  This recursive descent parser implements full expression precedence,
//  ternary conditionals, comma operators, and basic statements (let, print).
//
//  Follows Pratt parsing style for some constructs.
// ─────────────────────────────────────────────────────────────────────────────

#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include "Scanner\TokenType.h"
#include "Scanner\Token.h"
#include "ExpressionNode.h"
#include "Stmt.h"

class Parser
{
public:
    // ─────────────────────────────────────────────────────────────
    // Custom Exception for Syntax Errors
    // Raised when a parse failure occurs (used for recovery)
    // ─────────────────────────────────────────────────────────────
    class ParseError : public std::runtime_error
    {
    public:
        ParseError(const std::string& message) : std::runtime_error(message) {};
    };

private:
    // ─────────────────────────────────────────────────────────────
    // Parser State
    // ─────────────────────────────────────────────────────────────
    std::vector<Token> tokens;  // full list of tokens from the scanner
    int current = 0;            // current token being processed

    // ─────────────────────────────────────────────────────────────
    // Recursive Descent Parsing Methods (Expressions)
    // Ordered from lowest to highest precedence
    // ─────────────────────────────────────────────────────────────
    ExprPtr expression();      // entry point
    ExprPtr assignment();      // =
    ExprPtr conditional();     // ternary (?:)
    ExprPtr logicalOr();       // 'or' OR logical operator
    ExprPtr logicalAnd();      // 'and' AND logical operator
    ExprPtr comma();           // comma operator
    ExprPtr equality();        // ==, !=
    ExprPtr comparison();      // <, <=, >, >=
    ExprPtr term();            // +, -
    ExprPtr factor();          // *, /, %
    ExprPtr unary();           // !, -
    ExprPtr call();           // function call ()
    ExprPtr primary();         // literals, parens, identifiers
    ExprPtr lambda();         // anonymous/lambda expressions

    // ─────────────────────────────────────────────────────────────
    // Token Utility Helpers
    // ─────────────────────────────────────────────────────────────
    bool match(const std::vector<TokenType>& types);   // consume token if matches any in list
    bool check(TokenType type);                        // check if current token matches
    Token advance();                                   // consume and return current token
    Token consume(TokenType type, std::string message);// assert token type or error
    Token peek();                                      // look at current token
    Token previous();                                  // last consumed token
    bool isAtEnd();                                    // EOF check

    // ─────────────────────────────────────────────────────────────
    // Function call Helpers
    // ─────────────────────────────────────────────────────────────
    ExprPtr finishCall(ExprPtr callee);
    
    // ─────────────────────────────────────────────────────────────
    // Error Handling
    // ─────────────────────────────────────────────────────────────
    [[nodiscard]] Parser::ParseError error(Token token, std::string message); // raise ParseError
    void synchronize();                             // skip tokens after error to recover

    // ─────────────────────────────────────────────────────────────
    // Statement Parsers
    // ─────────────────────────────────────────────────────────────
    std::shared_ptr<Statement> declareStatement();      // entry point for top-level stmt
    std::shared_ptr<Statement> parseClassDeclaration();
    std::shared_ptr<Statement> parseVarDeclaration();   // `let` statement
    std::shared_ptr<Statement> parseFuncDeclaration(std::string kind);   // `func` statement
    std::shared_ptr<Statement> parseStatement();        // generic statement
    std::shared_ptr<Statement> ifStatement();           // 'if' condition statement
    std::shared_ptr<Statement> whileStatement();        // 'while' loop statement
    std::shared_ptr<Statement> returnStatement();        // 'return' statement
    std::shared_ptr<Statement> breakStatement();        // 'break' statement
    std::shared_ptr<Statement> continueStatement();      // 'continue' statement
    std::shared_ptr<Statement> forStatement();          // 'for' loop statement
    std::shared_ptr<Statement> printStatement();        // `print` statement
    std::shared_ptr<Statement> expressionStatement();   // expr followed by `;`
    std::vector<std::shared_ptr<Statement>> blockStatement(); // block of statements starts with '{' ends with '}'
    
    // ─────────────────────────────────────────────────────────────
    // AST Node Construction Helpers (Factory Methods)
    // Used to construct variant-wrapped expressions or statements
    // ─────────────────────────────────────────────────────────────
    template<typename T, typename... Args>
    std::shared_ptr<ExpressionNode> makeExpr(Args&&... args);

    template<typename T, typename... Args>
    std::shared_ptr<Statement> makeStmt(Args&&... args);

public:
    // ─────────────────────────────────────────────────────────────
    // Interface
    // ─────────────────────────────────────────────────────────────
    std::vector<std::shared_ptr<Statement>> parse();  // entry point from Interpreter
    Parser(std::vector<Token> tokens) : tokens(tokens) {};
    ~Parser() = default;
};
