#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  Parser.h – Token Stream → AST (Expressions & Statements)
// ─────────────────────────────────────────────────────────────────────────────
//  Implements a recursive-descent parser with Pratt-style precedence for
//  constructing the AST from the list of Tokens produced by the Scanner.
//  Handles expressions (with full operator precedence and ternaries)
//  and various statement types (let, if, while, for, return, etc.).
// ─────────────────────────────────────────────────────────────────────────────

#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include "Flint/Scanner/TokenType.h"  // TokenType enum for matching
#include "Flint/Scanner/Token.h"      // Token struct holding lexeme, type, literal
#include "Flint/ASTNodes/ExpressionNode.h"     // ExprPtr and expression node variants
#include "Flint/ASTNodes/Stmt.h"               // Statement variants

class Parser {
public:
    //──────────────────────────────────────────────────────────────────────────
    // ParseError: thrown on syntax errors, caught for synchronization
    //──────────────────────────────────────────────────────────────────────────
    class ParseError : public std::runtime_error {
    public:
        explicit ParseError(const std::string& message)
            : std::runtime_error(message) {}
    };

private:
    //──────────────────────────────────────────────────────────────────────────
    // Input tokens and cursor
    //──────────────────────────────────────────────────────────────────────────
    std::vector<Token> tokens;  // All tokens to process
    int current = 0;            // Index of next token to consume

    //──────────────────────────────────────────────────────────────────────────
    // Expression Parsers (lowest → highest precedence)
    //──────────────────────────────────────────────────────────────────────────
    ExprPtr expression();    // Entry point: parses comma-separated expressions
    ExprPtr assignment();    // Handles `=` and verifies l-value
    ExprPtr conditional();   // Ternary operator `?:`
    ExprPtr logicalOr();     // `or` operator
    ExprPtr logicalAnd();    // `and` operator
    ExprPtr comma();         // Comma operator (evaluates left, returns right)
    ExprPtr equality();      // `==`, `!=`
    ExprPtr comparison();    // `<`, `>`, `<=`, `>=`
    ExprPtr term();          // `+`, `-`
    ExprPtr factor();        // `*`, `/`, `%`
    ExprPtr unary();         // Prefix `!`, `-`
    ExprPtr call();          // Function calls and property access
    ExprPtr primary();       // Literals, grouping, identifiers, `this`, `super`
    ExprPtr lambda();        // Anonymous function expressions

    //──────────────────────────────────────────────────────────────────────────
    // Statement Parsers (top-level and nested)
    //──────────────────────────────────────────────────────────────────────────
    std::shared_ptr<Statement> declareStatement();     // `let`, `func`, `class` or fallback
    std::shared_ptr<Statement> parseClassDeclaration();
    std::shared_ptr<Statement> parseVarDeclaration();  // `let` statements
    std::shared_ptr<Statement> parseFuncDeclaration(std::string&& kind); // `func` or getter
    std::shared_ptr<Statement> parseStatement();       // Dispatch to specific stmts
    std::shared_ptr<Statement> ifStatement();          // `if` syntax
    std::shared_ptr<Statement> whileStatement();       // `while` loops
    std::shared_ptr<Statement> forStatement();         // `for` loops (desugared)
    std::shared_ptr<Statement> returnStatement();      // `return` in functions
    std::shared_ptr<Statement> breakStatement();       // `break` in loops
    std::shared_ptr<Statement> continueStatement();    // `continue` in loops
    std::shared_ptr<Statement> printStatement();       // `print` builtin
    std::shared_ptr<Statement> expressionStatement();  // Expressions as stmts
    std::vector<std::shared_ptr<Statement>> blockStatement(); // `{ ... }` block

    //──────────────────────────────────────────────────────────────────────────
    // Token Utilities
    //──────────────────────────────────────────────────────────────────────────
    bool match(const std::vector<TokenType>& types);   // If current matches any, consume
    bool check(TokenType type);                        // Peek check without consuming
    Token advance();                                   // Consume and return current
    Token consume(TokenType type, const std::string& message); // Assert type or throw
    Token peek() const;                                // Lookahead current token
    Token previous() const;                            // Last consumed token
    bool isAtEnd() const;                              // EOF reached?

    //──────────────────────────────────────────────────────────────────────────
    // Call Parsing Helper
    //──────────────────────────────────────────────────────────────────────────
    ExprPtr finishCall(ExprPtr callee); // Parse argument list and `)`

    //──────────────────────────────────────────────────────────────────────────
    // Error Handling & Recovery
    //──────────────────────────────────────────────────────────────────────────
    [[nodiscard]] ParseError error(const Token& token, const std::string& message); // Create error
    void synchronize();                   // Discard tokens until statement boundary

    //──────────────────────────────────────────────────────────────────────────
    // AST Factory Helpers (wrap nodes in shared_ptr)
    //──────────────────────────────────────────────────────────────────────────
    template<typename T, typename... Args>
    std::shared_ptr<ExpressionNode> makeExpr(Args&&... args);

    template<typename T, typename... Args>
    std::shared_ptr<Statement> makeStmt(Args&&... args);

public:
    //──────────────────────────────────────────────────────────────────────────
    // parse
    //──────────────────────────────────────────────────────────────────────────
    // Entry point: returns a vector of top-level Statements for interpretation.
    std::vector<std::shared_ptr<Statement>> parse();

    //──────────────────────────────────────────────────────────────────────────
    // Constructor
    //──────────────────────────────────────────────────────────────────────────
    explicit Parser(std::vector<Token> tokens)
        : tokens(std::move(tokens)) {}
};