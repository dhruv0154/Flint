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
        if (match({ TokenType::FUNC })) return parseFuncDeclaration("function");
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

    consume(TokenType::SEMICOLON, "Expected ';'.");
    return makeStmt<LetStmt>(name, initializer);
}

std::shared_ptr<Statement> Parser::parseFuncDeclaration(std::string kind)
{
    Token name = consume(TokenType::IDENTIFIER, "Expected " + kind + " name.");
    consume(TokenType::LEFT_PAREN, "Expected '(' at the start of " + kind + " name.");
    std::vector<Token> params;

    if(!check(TokenType::RIGHT_PAREN))
    {
        do
        {
            if(params.size() >= 255) error(peek(), "More than 255 arguments are not allowed.");

            params.emplace_back(consume(TokenType::IDENTIFIER, "Expected parameter name."));
        } while (match({ TokenType::COMMA }));
        
    }

    consume(TokenType::RIGHT_PAREN, "Expected ')' after function parameters.");

    consume(TokenType::LEFT_BRACE, "Expected '{' at the start of " + kind + " body.");

    std::vector<std::shared_ptr<Statement>> body = blockStatement();

    return makeStmt<FunctionStmt>(name, params, body);
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses any valid statement
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::parseStatement()
{
    if (match({ TokenType::IF }))
        return ifStatement();
    else if (match({ TokenType::FOR }))
        return forStatement();
    else if (match({ TokenType::WHILE }))
        return whileStatement();
    else if (match({ TokenType::RETURN }))
        return returnStatement();
    else if (match({ TokenType::BREAK }))
        return breakStatement();
    else if (match({ TokenType::CONTINUE }))
        return continueStatement();
    else if (match({ TokenType::LEFT_BRACE })) 
        return makeStmt<BlockStmt>(blockStatement());
    return expressionStatement();
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses a `if` statement
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::ifStatement()
{
    consume(TokenType::LEFT_PAREN, "Expected a '(' at the start of 'if' condition.");
    ExprPtr condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expected a ')' at the end of 'if' condition.");

    std::shared_ptr<Statement> thenBranch = parseStatement();
    std::shared_ptr<Statement> elseBranch = nullptr;
    if(match({ TokenType::ELSE })) elseBranch = parseStatement();

    return makeStmt<IfStmt>(condition, thenBranch, elseBranch);
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses a `while` statement
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::whileStatement()
{
    consume(TokenType::LEFT_PAREN, 
        "Expected '(' at the start of while condition.");

    ExprPtr condition = expression();
    
    consume(TokenType::RIGHT_PAREN, 
        "Expected ')' at the end of while condition.");
    
    std::shared_ptr<Statement> body = parseStatement();

    return makeStmt<WhileStmt>(condition, body);
}

std::shared_ptr<Statement> Parser::returnStatement()
{
    Token keyword = previous();
    ExprPtr val = nullptr;
    if(!check(TokenType::SEMICOLON)) val = expression();
    consume(TokenType::SEMICOLON, "Expected ';' at the end of return value.");
    return makeStmt<ReturnStmt>(keyword, val);
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses a `break` statement
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::breakStatement()
{
    Token keyword = previous();
    consume(TokenType::SEMICOLON, "Expected ';' at the end of break statement");
    return makeStmt<BreakStmt>(keyword);
}

std::shared_ptr<Statement> Parser::continueStatement()
{
    Token keyword = previous();
    consume(TokenType::SEMICOLON, "Expected ';' at the end of continue statement");
    return makeStmt<ContinueStmt>(keyword);
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses a `for` statement
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::forStatement()
{
    consume(TokenType::LEFT_PAREN, 
        "Expected '(' at the start of for clauses.");

    std::shared_ptr<Statement> initializer;
    if(match({ TokenType::SEMICOLON })) initializer = nullptr;
    else if(match({ TokenType::LET })) initializer = parseVarDeclaration();
    else initializer = expressionStatement();

    ExprPtr condition = nullptr;
    if(!check(TokenType::SEMICOLON)) condition = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after for loop condition");

    ExprPtr increment = nullptr;
    if(!check(TokenType::RIGHT_PAREN)) increment = expression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' at the end of for clauses");

    std::shared_ptr<Statement> body = parseStatement();

    if(increment)
    {
        body = makeStmt<BlockStmt>(std::vector<std::shared_ptr<Statement>>
            { makeStmt<TryCatchContinueStmt>(body), makeStmt<ExpressionStmt>(increment) });
    }
    else
    {
        body = makeStmt<TryCatchContinueStmt>(body);
    }

    if(!condition) condition = makeExpr<Literal>(true);
    body = makeStmt<WhileStmt>(condition, body);

    if(initializer)
    {
        body = makeStmt<BlockStmt>(std::vector<std::shared_ptr<Statement>>
            { initializer, body });
    }
    return body;
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses a block of statements (starts with '{' ends with '}')
// ─────────────────────────────────────────────────────────────────────────────
std::vector<std::shared_ptr<Statement>> Parser::blockStatement()
{
    std::vector<std::shared_ptr<Statement>> statements;

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) 
        statements.emplace_back(declareStatement());

    consume(TokenType::RIGHT_BRACE, "Expect '}' at the end of block.");
    return statements;
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
// Parses full expressions starting with ternary conditionals
// ─────────────────────────────────────────────────────────────────────────────
ExprPtr Parser::expression()
{
    return comma();
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses comma-separated expressions: a, b, c (evaluates all, returns rightmost)
// ─────────────────────────────────────────────────────────────────────────────
ExprPtr Parser::comma()
{
    if (match({ TokenType::COMMA })) {
        Token op = previous();
        error(op, "Missing left-hand operand before ','.");
        return assignment();
    }

    auto expr = assignment();
    while (match({ TokenType::COMMA })) {
        Token op = previous();
        auto right = assignment();
        expr = makeExpr<Binary>(expr, op, right);
    }
    return expr;
}

ExprPtr Parser::assignment()
{
    ExprPtr expr = conditional();

    if(match({ TokenType::EQUAL }))
    {
        Token equals = previous();
        ExprPtr value = assignment();
        if(std::holds_alternative<Variable>(*expr))
        {
            Token name = std::get<Variable>(*expr).name;
            return makeExpr<Assignment>(name, value);
        }
        throw error(equals, "Invalid assignment target.");
    }
    return expr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses ternary conditionals: a ? b : c
// Right-associative structure is enforced
// ─────────────────────────────────────────────────────────────────────────────
ExprPtr Parser::conditional()
{
    ExprPtr expr = logicalOr();

    if (match({ TokenType::QUESTION_MARK }))
    {
        ExprPtr thenBranch = conditional();
        consume(TokenType::COLON, "Expected ':' after then branch of ternary operator.");
        ExprPtr elseBranch = conditional();
        expr = makeExpr<Conditional>(expr, thenBranch, elseBranch);
    }

    return expr;
}

ExprPtr Parser::logicalOr()
{
    ExprPtr expr = logicalAnd();

    while (match({ TokenType::OR }))
    {
        Token op = previous();
        ExprPtr right = logicalAnd();
        expr = makeExpr<Logical>(expr, op, right);
    }
    
    return expr;
}

ExprPtr Parser::logicalAnd()
{
    ExprPtr expr = equality();

    while (match({ TokenType::AND }))
    {
        Token op = previous();
        ExprPtr right = equality();
        expr = makeExpr<Logical>(expr, op, right);
    }
    
    return expr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses equality expressions: == and !=
// ─────────────────────────────────────────────────────────────────────────────
ExprPtr Parser::equality()
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
ExprPtr Parser::comparison()
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
ExprPtr Parser::term()
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
ExprPtr Parser::factor()
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
ExprPtr Parser::unary()
{
    if (match({ TokenType::BANG, TokenType::MINUS })) {
        Token op = previous();
        ExprPtr right = unary();
        return makeExpr<Unary>(op, right);
    }

    return call();
}

ExprPtr Parser::call()
{
    ExprPtr expr = primary();

    while (true) 
    {
        if (match({ TokenType::LEFT_PAREN })) 
        {
            expr = finishCall(expr);
        } 
        else if (check(TokenType::STRING) || check(TokenType::NUMBER) || check(TokenType::IDENTIFIER)) 
        {
            // This means we're trying to call without a left paren
            throw error(peek(), "Expected '(' after function name to start a call.");
        } 
        else 
        {
            break;
        }
    }
    return expr;
}

ExprPtr Parser::finishCall(ExprPtr callee)
{
    std::vector<ExprPtr> arguments;

    if(!check(TokenType::RIGHT_PAREN))
    {
        do
        {
            if(arguments.size() >= 255) error(peek(), "More than 255 arguments are not allowed.");
            arguments.emplace_back(assignment());
        } while (match({ TokenType::COMMA }));
    }

    Token paren =  consume(TokenType::RIGHT_PAREN, 
        "Expect ')' at the end of function arguments");
    
    return makeExpr<Call>(callee, paren, arguments);
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses primary literals: true, false, nothing, numbers, strings, identifiers
// Also handles parenthesized expressions
// ─────────────────────────────────────────────────────────────────────────────
ExprPtr Parser::primary()
{
    if (match({ TokenType::FALSE }))   return makeExpr<Literal>(false);
    if (match({ TokenType::TRUE }))    return makeExpr<Literal>(true);
    if (match({ TokenType::NOTHING })) return makeExpr<Literal>(std::monostate{});
    if (match({ TokenType::NUMBER, TokenType::STRING }))
        return makeExpr<Literal>(previous().literal);
    if (match({ TokenType::IDENTIFIER }))
    {
        Token name = previous();

        return makeExpr<Variable>(name);
    }

    if (match({ TokenType::LEFT_PAREN })) {
        ExprPtr expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expected a ')'.");
        return makeExpr<Grouping>(expr);
    }

    throw error(peek(), "Expected an expression.");
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
ExprPtr Parser::makeExpr(Args&&... args) 
{
    return std::make_shared<ExpressionNode>(T(std::forward<Args>(args)...));
}

template<typename T, typename... Args>
std::shared_ptr<Statement> Parser::makeStmt(Args&&... args) 
{
    return std::make_shared<Statement>(T(std::forward<Args>(args)...));
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