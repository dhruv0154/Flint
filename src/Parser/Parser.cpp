#include "Parser\Parser.h"
#include "Flint\Flint.h"
#include "Stmt.h"

// ─────────────────────────────────────────────────────────────────────────────
// Top-level entry point to parse a list of statements (like a full program)
// Keeps consuming statements until EOF
// ─────────────────────────────────────────────────────────────────────────────
std::vector<std::shared_ptr<Statement>> Parser::parse()
{
    std::vector<std::shared_ptr<Statement>> statements;

    while (!isAtEnd())
        statements.push_back(declareStatement());

    return statements;
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses either a variable declaration or a general statement.
// Uses try-catch to recover from syntax errors and continue parsing.
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::declareStatement()
{
    try
    {
        if (match({ TokenType::LET })) return parseVarDeclaration();
        return parseStatement();
    }
    catch (ParseError error)
    {
        synchronize(); // Skips to a safe place
        return nullptr;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses variable declarations:
//     let x = 3;
//     let y;
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::parseVarDeclaration()
{
    Token name = consume(TokenType::IDENTIFIER, "Expected an indentifier.");

    ExprPtr initializer = nullptr;
    if (match({ TokenType::EQUAL })) initializer = expression();

    consume(TokenType::SEMICOLON, "Expected ';' at the end of statement.");
    return makeStmt<LetStmt>(name, initializer);
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses any valid statement: print or expression statement for now
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::parseStatement()
{
    if (match({ TokenType::PRINT })) return printStatement();
    return expressionStatement();
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses full expressions starting with ternary conditionals
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<ExpressionNode> Parser::expression()
{
    return conditional();
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses ternary conditionals: a ? b : c
// Right-associative structure is enforced
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<ExpressionNode> Parser::conditional()
{
    std::shared_ptr<ExpressionNode> expr = comma();

    if (match({ TokenType::QUESTION_MARK }))
    {
        std::shared_ptr<ExpressionNode> thenBranch = conditional();
        consume(TokenType::COLON, "Expected ':' after then branch of ternary operator.");
        std::shared_ptr<ExpressionNode> elseBranch = conditional();
        expr = makeExpr<Conditional>(expr, thenBranch, elseBranch);
    }

    return expr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses comma-separated expressions: a, b, c (evaluates all, returns rightmost)
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<ExpressionNode> Parser::comma()
{
    if (match({ TokenType::COMMA })) {
        Token op = previous();
        error(op, "Missing left-hand operand before ','.");
        return equality();
    }

    auto expr = equality();
    while (match({ TokenType::COMMA })) {
        Token op = previous();
        auto right = equality();
        expr = makeExpr<Binary>(expr, op, right);
    }
    return expr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses equality expressions: == and !=
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<ExpressionNode> Parser::equality()
{
    if (match({ TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL })) {
        Token op = previous();
        error(op, "Missing left-hand operand before '" + op.lexeme + "'.");
        return comparison();
    }

    auto expr = comparison();
    while (match({ TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL })) {
        Token op = previous();
        auto right = comparison();
        expr = makeExpr<Binary>(expr, op, right);
    }
    return expr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses comparisons: <, <=, >, >=
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<ExpressionNode> Parser::comparison()
{
    if (match({ TokenType::LESS, TokenType::LESS_EQUAL, TokenType::GREATER, TokenType::GREATER_EQUAL })) {
        Token op = previous();
        error(op, "Missing left-hand operand before '" + op.lexeme + "'.");
        return term();
    }

    auto expr = term();
    while (match({ TokenType::LESS, TokenType::LESS_EQUAL, TokenType::GREATER, TokenType::GREATER_EQUAL })) {
        Token op = previous();
        auto right = term();
        expr = makeExpr<Binary>(expr, op, right);
    }
    return expr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses addition and subtraction: + and -
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<ExpressionNode> Parser::term()
{
    auto expr = factor();
    while (match({ TokenType::PLUS, TokenType::MINUS })) {
        Token op = previous();
        auto right = factor();
        expr = makeExpr<Binary>(expr, op, right);
    }
    return expr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses multiplication, division, and modulo: *, /, %
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<ExpressionNode> Parser::factor()
{
    if (match({ TokenType::STAR, TokenType::SLASH, TokenType::MODULO })) {
        Token op = previous();
        error(op, "Missing left-hand operand before '" + op.lexeme + "'.");
        return unary();
    }

    auto expr = unary();
    while (match({ TokenType::STAR, TokenType::SLASH, TokenType::MODULO })) {
        Token op = previous();
        auto right = unary();
        expr = makeExpr<Binary>(expr, op, right);
    }
    return expr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses unary expressions: - and !
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<ExpressionNode> Parser::unary()
{
    if (match({ TokenType::BANG, TokenType::MINUS })) {
        Token op = previous();
        std::shared_ptr<ExpressionNode> right = unary();
        return makeExpr<Unary>(op, right);
    }

    return primary();
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses primary literals: true, false, nothing, numbers, strings, identifiers
// Also handles parenthesized expressions
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<ExpressionNode> Parser::primary()
{
    if (match({ TokenType::FALSE }))   return makeExpr<Literal>(false);
    if (match({ TokenType::TRUE }))    return makeExpr<Literal>(true);
    if (match({ TokenType::NOTHING })) return makeExpr<Literal>(std::monostate{});
    if (match({ TokenType::NUMBER, TokenType::STRING }))
        return makeExpr<Literal>(previous().literal);
    if (match({ TokenType::IDENTIFIER }))
        return makeExpr<Variable>(previous());

    if (match({ TokenType::LEFT_PAREN })) {
        std::shared_ptr<ExpressionNode> expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expected a ')'.");
        return makeExpr<Grouping>(expr);
    }

    throw error(peek(), "Expected an expression.");
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses a `print` statement
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::printStatement()
{
    ExprPtr val = expression();
    consume(TokenType::SEMICOLON, "';' expected.");
    return makeStmt<PrintStmt>(val);
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses a generic expression statement (ends with `;`)
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::expressionStatement()
{
    ExprPtr expr = expression();
    consume(TokenType::SEMICOLON, "';' expected.");
    return makeStmt<ExpressionStmt>(expr);
}

// ─────────────────────────────────────────────────────────────────────────────
// Utility methods for matching/parsing tokens
// ─────────────────────────────────────────────────────────────────────────────

bool Parser::match(const std::vector<TokenType>& types)
{
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::check(TokenType type)
{
    if (isAtEnd()) return false;
    return peek().type == type;
}

Token Parser::advance()
{
    if (!isAtEnd()) current++;
    return previous();
}

Token Parser::consume(TokenType type, std::string message)
{
    if (check(type)) return advance();
    throw error(peek(), message);
}

// ─────────────────────────────────────────────────────────────────────────────
// Error Recovery: skip tokens until we find a good restart point
// ─────────────────────────────────────────────────────────────────────────────
void Parser::synchronize()
{
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;

        switch (previous().type) {
            case TokenType::CLASS:
            case TokenType::FUNC:
            case TokenType::LET:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::RETURN:
                return;
        }

        advance();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// AST Construction Helpers
// ─────────────────────────────────────────────────────────────────────────────
template<typename T, typename... Args>
std::shared_ptr<ExpressionNode> Parser::makeExpr(Args&&... args) 
{
    return std::make_shared<ExpressionNode>(T{std::forward<Args>(args)...});
}

template<typename T, typename... Args>
std::shared_ptr<Statement> Parser::makeStmt(Args&&... args) 
{
    return std::make_shared<Statement>(T{std::forward<Args>(args)...});
}

// ─────────────────────────────────────────────────────────────────────────────
// Error Reporting Utility
// ─────────────────────────────────────────────────────────────────────────────
Parser::ParseError Parser::error(Token token, std::string message)
{
    Flint::error(token, message);
    return ParseError(message);
}

// ─────────────────────────────────────────────────────────────────────────────
// Peek/Pull Tokens
// ─────────────────────────────────────────────────────────────────────────────
bool Parser::isAtEnd()
{
    return peek().type == TokenType::END_OF_FILE;
}

Token Parser::peek()
{
    return tokens.at(current);
}

Token Parser::previous()
{
    return tokens.at(current - 1);
}