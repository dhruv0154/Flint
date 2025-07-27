#include "Flint/Parser/Parser.h"
#include "Flint/Flint.h"
#include "Flint/ASTNodes/Stmt.h"

// ─────────────────────────────────────────────────────────────────────────────
// Entry point: parse a sequence of statements until EOF.
// Returns a vector of Statement AST nodes for interpretation.
// ─────────────────────────────────────────────────────────────────────────────
std::vector<std::shared_ptr<Statement>> Parser::parse()
{
    std::vector<std::shared_ptr<Statement>> statements;
    // Keep consuming top‑level declarations/statements until we hit END_OF_FILE
    while (!isAtEnd()) {
        statements.push_back(declareStatement());
    }
    return statements;
}

// ─────────────────────────────────────────────────────────────────────────────
// Top‑level “declaration or statement” parser.
// Distinguishes class/func/let from other statements.
// Uses exception-based error recovery to skip bad tokens.
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::declareStatement()
{
    try {
        if (match({ TokenType::CLASS }))
            return parseClassDeclaration();
        if (match({ TokenType::FUNC }))
            return parseFuncDeclaration("function");
        if (match({ TokenType::LET }) && check(TokenType::IDENTIFIER))
            return parseVarDeclaration();
        // Fallback: parse as a normal statement
        return parseStatement();
    } catch (ParseError error) {
        // On syntax error, skip tokens until next safe point
        synchronize();
        return nullptr;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// class Foo { ... }  
// Parse class name, then member declarations until '}'.
// Collects instance vs. static methods separately.
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::parseClassDeclaration()
{
    // Require an identifier for the class name
    Token name = consume(TokenType::IDENTIFIER, "Expected an identifier for class name.");

    ExprPtr superClass = nullptr;

    if(match({ TokenType::LESS })) {
        consume(TokenType::IDENTIFIER, "Expected an identifier for super class name.");
        superClass = makeExpr<Variable>(previous());
    }

    consume(TokenType::LEFT_BRACE, "Expected '{' at the start of class body.");

    std::vector<std::shared_ptr<Statement>> instanceMethods;
    std::vector<std::shared_ptr<Statement>> classMethods;

    // Loop until we see the closing brace
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        if (match({ TokenType::CLASS })) {
            // static method
            classMethods.push_back(parseFuncDeclaration("method"));
        } else {
            // instance method
            instanceMethods.push_back(parseFuncDeclaration("method"));
        }
    }

    consume(TokenType::RIGHT_BRACE, "Expected '}' at the end of class body.");
    // Build a ClassStmt node with both method lists
    return makeStmt<ClassStmt>(name, superClass, instanceMethods, classMethods);
}

// ─────────────────────────────────────────────────────────────────────────────
// let a = 1, b, c = foo();
// Parses one or more comma‑separated declarations.
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::parseVarDeclaration()
{
    std::vector<std::pair<Token, ExprPtr>> declarations;

    do {
        // Name of the variable
        Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");
        ExprPtr initializer = nullptr;

        // Optional initializer after '='
        if (match({ TokenType::EQUAL })) {
            initializer = assignment();
        }

        declarations.emplace_back(name, initializer);
        // If there's a comma, continue parsing more var names
    } while (match({ TokenType::COMMA }));

    // Expect a semicolon to terminate the declaration
    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return makeStmt<LetStmt>(declarations);
}

// ─────────────────────────────────────────────────────────────────────────────
// function foo(...) { ... }  
// or a “getter” if no parens follow the name.
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::parseFuncDeclaration(std::string&& kind)
{
    Token name = consume(TokenType::IDENTIFIER, "Expected " + kind + " name.");

    std::vector<Token> params;
    bool isGetter = false;

    // If the next token isn't '(', treat this as a getter/setter
    if (!check(TokenType::LEFT_PAREN)) {
        isGetter = true;
        kind = "getter/setter";
    }

    if (!isGetter) {
        // Parse parameter list in parentheses
        consume(TokenType::LEFT_PAREN, "Expected '(' after " + kind + " name.");
        if (!check(TokenType::RIGHT_PAREN)) {
            do {
                if (params.size() >= 255)
                    error(peek(), "Cannot have more than 255 parameters.");
                params.emplace_back(
                    consume(TokenType::IDENTIFIER, "Expected parameter name."));
            } while (match({ TokenType::COMMA }));
        }
        consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters.");
    }

    // Parse the function body as a block
    consume(TokenType::LEFT_BRACE, "Expected '{' to start " + kind + " body.");
    auto body = blockStatement();

    return makeStmt<FunctionStmt>(name, params, body, isGetter);
}

// ─────────────────────────────────────────────────────────────────────────────
// Dispatch based on leading token: if/for/while/etc.
// Otherwise parse as an expression statement.
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::parseStatement()
{
    if      (match({ TokenType::IF }))       return ifStatement();
    else if (match({ TokenType::FOR }))      return forStatement();
    else if (match({ TokenType::WHILE }))    return whileStatement();
    else if (match({ TokenType::RETURN }))   return returnStatement();
    else if (match({ TokenType::BREAK }))    return breakStatement();
    else if (match({ TokenType::CONTINUE })) return continueStatement();
    else if (match({ TokenType::LEFT_BRACE }))
        return makeStmt<BlockStmt>(blockStatement());

    return expressionStatement();
}

// ─────────────────────────────────────────────────────────────────────────────
// if (cond) thenBranch else elseBranch
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::ifStatement()
{
    consume(TokenType::LEFT_PAREN,  "Expected '(' after 'if'.");
    ExprPtr condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after if condition.");

    auto thenBranch = parseStatement();
    std::shared_ptr<Statement> elseBranch = nullptr;
    if (match({ TokenType::ELSE })) {
        elseBranch = parseStatement();
    }

    return makeStmt<IfStmt>(condition, thenBranch, elseBranch);
}

// ─────────────────────────────────────────────────────────────────────────────
// while (cond) body
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::whileStatement()
{
    consume(TokenType::LEFT_PAREN,  "Expected '(' after 'while'.");
    ExprPtr condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after while condition.");

    auto body = parseStatement();
    return makeStmt<WhileStmt>(condition, body);
}

// ─────────────────────────────────────────────────────────────────────────────
// return expr? ;
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::returnStatement()
{
    Token keyword = previous();
    ExprPtr value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = expression();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after return value.");
    return makeStmt<ReturnStmt>(keyword, value);
}

// ─────────────────────────────────────────────────────────────────────────────
// break ;
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::breakStatement()
{
    Token keyword = previous();
    consume(TokenType::SEMICOLON, "Expected ';' after break.");
    return makeStmt<BreakStmt>(keyword);
}

// ─────────────────────────────────────────────────────────────────────────────
// continue ;
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::continueStatement()
{
    Token keyword = previous();
    consume(TokenType::SEMICOLON, "Expected ';' after continue.");
    return makeStmt<ContinueStmt>(keyword);
}

// ─────────────────────────────────────────────────────────────────────────────
// for ( init; cond; incr ) body
// Transforms into:
// { init; while(cond) { body; incr; } }
// Wraps body in TryCatchContinue to handle continue properly.
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::forStatement()
{
    consume(TokenType::LEFT_PAREN,  "Expected '(' after 'for'.");

    // Parse initializer (let, expression, or empty)
    std::shared_ptr<Statement> initializer;
    if      (match({ TokenType::SEMICOLON })) initializer = nullptr;
    else if (match({ TokenType::LET      })) initializer = parseVarDeclaration();
    else                                       initializer = expressionStatement();

    // Parse loop condition
    ExprPtr condition = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        condition = expression();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after loop condition.");

    // Parse increment expression
    ExprPtr increment = nullptr;
    if (!check(TokenType::RIGHT_PAREN)) {
        increment = expression();
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after for clauses.");

    // Parse the loop body
    auto body = parseStatement();

    // If there's an increment, execute it after each iteration
    if (increment) {
        body = makeStmt<BlockStmt>(
            std::vector<std::shared_ptr<Statement>>{
                makeStmt<TryCatchContinueStmt>(body),
                makeStmt<ExpressionStmt>(increment)
            }
        );
    } else {
        // Still catch continues so they don't skip the increment
        body = makeStmt<TryCatchContinueStmt>(body);
    }

    // Default condition to true if omitted
    if (!condition) {
        condition = makeExpr<Literal>(true);
    }
    // Build a while loop around the body
    body = makeStmt<WhileStmt>(condition, body);

    // If there was an initializer, wrap everything in a block
    if (initializer) {
        body = makeStmt<BlockStmt>(
            std::vector<std::shared_ptr<Statement>>{ initializer, body }
        );
    }
    return body;
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses a sequence '{ ... }' into a vector of statements.
// ─────────────────────────────────────────────────────────────────────────────
std::vector<std::shared_ptr<Statement>> Parser::blockStatement()
{
    std::vector<std::shared_ptr<Statement>> statements;
    // Keep parsing declarations/statements until '}'
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(declareStatement());
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' at end of block.");
    return statements;
}

// ─────────────────────────────────────────────────────────────────────────────
// expr;
// Parses a standalone expression followed by semicolon.
// ─────────────────────────────────────────────────────────────────────────────
std::shared_ptr<Statement> Parser::expressionStatement()
{
    ExprPtr expr = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression.");
    return makeStmt<ExpressionStmt>(expr);
}

// ─────────────────────────────────────────────────────────────────────────────
// Full expression parser entry: handles comma operators.
// ─────────────────────────────────────────────────────────────────────────────
ExprPtr Parser::expression()
{
    return comma();
}

// ─────────────────────────────────────────────────────────────────────────────
// comma: left , right → Binary with COMMA, allowing chaining.
// Reports missing left operand error if leading comma.
// ─────────────────────────────────────────────────────────────────────────────
ExprPtr Parser::comma()
{
    if (match({ TokenType::COMMA })) {
        // Leading comma is a syntax error
        Token op = previous();
        error(op, "Missing left-hand operand before ','.");
        return assignment();
    }

    ExprPtr expr = assignment();
    // While we see commas, build a left‑folded Binary chain
    while (match({ TokenType::COMMA })) {
        Token op = previous();
        auto right = assignment();
        expr = makeExpr<Binary>(expr, op, right);
    }
    return expr;
}

// ─────────────────────────────────────────────────────────────────────────────
// assignment: handles a = b or property sets.
// Right-associative, so `a = b = c` binds as `a = (b = c)`.
// ─────────────────────────────────────────────────────────────────────────────
ExprPtr Parser::assignment()
{
    ExprPtr expr = conditional();

    if (match({ TokenType::EQUAL })) {
        Token equals = previous();
        ExprPtr value = assignment();

        // Simple variable assignment
        if (std::holds_alternative<Variable>(*expr)) {
            Token name = std::get<Variable>(*expr).name;
            return makeExpr<Assignment>(name, value);
        }
        // Property assignment: obj.prop = val
        else if (std::holds_alternative<Get>(*expr)) {
            auto get = std::get<Get>(*expr);
            return makeExpr<Set>(get.object, get.name, value);
        }

        // Array element assigment: a[1] = 5;
        else if (std::holds_alternative<GetIndex>(*expr)) {
            auto getIndex = std::get<GetIndex>(*expr);
            return makeExpr<SetIndex>(getIndex.array, getIndex.index, value, getIndex.bracket);
        }

        // Otherwise it's an invalid target
        throw error(equals, "Invalid assignment target.");
    }

    return expr;
}

// ─────────────────────────────────────────────────────────────────────────────
// conditional (ternary): a ? b : c, right-associative.
// ─────────────────────────────────────────────────────────────────────────────
ExprPtr Parser::conditional()
{
    ExprPtr expr = logicalOr();

    if (match({ TokenType::QUESTION_MARK })) {
        ExprPtr thenBranch = conditional();
        consume(TokenType::COLON, "Expected ':' after then branch.");
        ExprPtr elseBranch = conditional();
        expr = makeExpr<Conditional>(expr, thenBranch, elseBranch);
    }

    return expr;
}

// ─────────────────────────────────────────────────────────────────────────────
// logicalOr: left || right ... short‑circuit at runtime.
// ─────────────────────────────────────────────────────────────────────────────
ExprPtr Parser::logicalOr()
{
    ExprPtr expr = logicalAnd();
    while (match({ TokenType::OR })) {
        Token op = previous();
        ExprPtr right = logicalAnd();
        expr = makeExpr<Logical>(expr, op, right);
    }
    return expr;
}

// ─────────────────────────────────────────────────────────────────────────────
// logicalAnd: left && right ... short‑circuit at runtime.
// ─────────────────────────────────────────────────────────────────────────────
ExprPtr Parser::logicalAnd()
{
    ExprPtr expr = equality();
    while (match({ TokenType::AND })) {
        Token op = previous();
        ExprPtr right = equality();
        expr = makeExpr<Logical>(expr, op, right);
    }
    return expr;
}

// ─────────────────────────────────────────────────────────────────────────────
// equality: == and !=, left associative.
// ─────────────────────────────────────────────────────────────────────────────
ExprPtr Parser::equality()
{
    ExprPtr expr = comparison();
    while (match({ TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL })) {
        Token op = previous();
        ExprPtr right = comparison();
        expr = makeExpr<Binary>(expr, op, right);
    }
    return expr;
}

// ─────────────────────────────────────────────────────────────────────────────
// comparison: <, <=, >, >=
// Provides an early-error if operator appears without left operand.
// ─────────────────────────────────────────────────────────────────────────────
ExprPtr Parser::comparison()
{
    // Leading comparison operator ⇒ error
    if (match({ TokenType::LESS, TokenType::LESS_EQUAL,
                TokenType::GREATER, TokenType::GREATER_EQUAL })) {
        Token op = previous();
        error(op, "Missing left-hand operand before '" + op.lexeme + "'.");
        return term();
    }

    ExprPtr expr = term();
    while (match({ TokenType::LESS, TokenType::LESS_EQUAL,
                   TokenType::GREATER, TokenType::GREATER_EQUAL })) {
        Token op    = previous();
        ExprPtr right = term();
        expr = makeExpr<Binary>(expr, op, right);
    }
    return expr;
}

// ─────────────────────────────────────────────────────────────────────────────
// term: + and -  
// ─────────────────────────────────────────────────────────────────────────────
ExprPtr Parser::term()
{
    ExprPtr expr = factor();
    while (match({ TokenType::PLUS, TokenType::MINUS })) {
        Token op    = previous();
        ExprPtr right = factor();
        expr = makeExpr<Binary>(expr, op, right);
    }
    return expr;
}

// ─────────────────────────────────────────────────────────────────────────────
// factor: *, /, %  
// ─────────────────────────────────────────────────────────────────────────────
ExprPtr Parser::factor()
{
    // Early error if operator without left operand
    if (match({ TokenType::STAR, TokenType::SLASH, TokenType::MODULO })) {
        Token op = previous();
        error(op, "Missing left-hand operand before '" + op.lexeme + "'.");
        return unary();
    }

    ExprPtr expr = unary();
    while (match({ TokenType::STAR, TokenType::SLASH, TokenType::MODULO })) {
        Token op    = previous();
        ExprPtr right = unary();
        expr = makeExpr<Binary>(expr, op, right);
    }
    return expr;
}

// ─────────────────────────────────────────────────────────────────────────────
// unary: '!' or '-' binds tighter than multiplication.
// ─────────────────────────────────────────────────────────────────────────────
ExprPtr Parser::unary()
{
    if (match({ TokenType::BANG, TokenType::MINUS })) {
        Token op    = previous();
        ExprPtr right = unary();        // recursive to allow multiple '!!a'
        return makeExpr<Unary>(op, right);
    }
    return call();
}

// ─────────────────────────────────────────────────────────────────────────────
// call: parses function calls and property accesses:
//   - foo(...)
//   - obj.prop
// Loops to allow chaining: foo().bar().baz()
// ─────────────────────────────────────────────────────────────────────────────
ExprPtr Parser::call()
{
    ExprPtr expr = primary();

    while (true) {
        if (match({ TokenType::LEFT_PAREN })) {
            expr = finishCall(expr);
        }
        else if (match({ TokenType::DOT })) {
            // property access: obj.identifier
            Token name = consume(TokenType::IDENTIFIER,
                                 "Expected property name after '.'.");
            expr = makeExpr<Get>(expr, name);
        }
        else if (match({ TokenType::LEFT_BRACKET })) {
            Token bracket = previous();
            ExprPtr indexExpr = expression();
            consume(TokenType::RIGHT_BRACKET, "Expected ']' after index.");
            expr = makeExpr<GetIndex>(expr, indexExpr, bracket);
        }
        else {
            break;
        }
    }

    // If an identifier/string/number follows without '(', that's a missing '(' error
    if (check(TokenType::IDENTIFIER) || check(TokenType::STRING) || check(TokenType::NUMBER)) {
        throw error(peek(), "Expected '(' after function name.");
    }
    return expr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Parse the argument list after seeing '('.  
// Ensures we don't exceed 255 args, then close with ')'.
// ─────────────────────────────────────────────────────────────────────────────
ExprPtr Parser::finishCall(ExprPtr callee)
{
    std::vector<ExprPtr> arguments;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            if (arguments.size() >= 255)
                error(peek(), "Cannot have more than 255 arguments.");
            arguments.push_back(assignment());
        } while (match({ TokenType::COMMA }));
    }
    Token paren = consume(TokenType::RIGHT_PAREN,
                          "Expected ')' after arguments.");
    return makeExpr<Call>(callee, paren, arguments);
}

// ─────────────────────────────────────────────────────────────────────────────
// primary: the atoms of our grammar:
//   literals, identifiers, 'this', lambdas, and parenthesized exprs.
// ─────────────────────────────────────────────────────────────────────────────
ExprPtr Parser::primary()
{
    if (match({ TokenType::FALSE }))   return makeExpr<Literal>(false);
    if (match({ TokenType::TRUE  }))   return makeExpr<Literal>(true);
    if (match({ TokenType::NOTHING })) return makeExpr<Literal>(std::monostate{});
    if (match({ TokenType::NUMBER, TokenType::STRING }))
        return makeExpr<Literal>(previous().literal);
    if (match({ TokenType::FUNC }))    return lambda();
    if (match({ TokenType::THIS }))    return makeExpr<This>(previous());
    if (match({ TokenType::SUPER })) {
        Token keyword = previous();
        consume(TokenType::DOT, "Expected '.' after 'super'.");
        Token method = consume(TokenType::IDENTIFIER, 
            "Expected an identifier for super class method name after '.'");
        return makeExpr<Super>(keyword, method);
    }
    if (match({ TokenType::LEFT_BRACKET })) {
        std::vector<ExprPtr> elements;

        if(!check(TokenType::RIGHT_BRACKET)) {
            do {
                elements.push_back(assignment());
            } while (match({ TokenType::COMMA })); 
        }

        consume(TokenType::RIGHT_BRACKET, "Expected ']' at the end of array elements.");
        return makeExpr<Array>(elements);
    }
    if (match({ TokenType::IDENTIFIER })) {
        Token name = previous();
        return makeExpr<Variable>(name);
    }
    if (match({ TokenType::LEFT_PAREN })) {
        // grouping: '( expr )'
        ExprPtr expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression.");
        return makeExpr<Grouping>(expr);
    }

    throw error(peek(), "Expected an expression.");
}

// ─────────────────────────────────────────────────────────────────────────────
// Parses a lambda literal: func(params) { body }
// ─────────────────────────────────────────────────────────────────────────────
ExprPtr Parser::lambda()
{
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'func'.");
    std::vector<Token> params;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            if (params.size() >= 255)
                error(peek(), "Cannot have more than 255 parameters.");
            params.push_back(
                consume(TokenType::IDENTIFIER, "Expected parameter name."));
        } while (match({ TokenType::COMMA }));
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters.");
    consume(TokenType::LEFT_BRACE,  "Expected '{' before lambda body.");
    auto body = blockStatement();
    // Lambdas have no name, so pass nullopt
    auto fnStmt = std::make_shared<FunctionStmt>(std::nullopt, params, body);
    return makeExpr<Lambda>(fnStmt);
}

// ─────────────────────────────────────────────────────────────────────────────
// match/check/advance/consume implement the basic token‐stream API.
// ─────────────────────────────────────────────────────────────────────────────
bool Parser::match(const std::vector<TokenType>& types)
{
    for (auto t : types) {
        if (check(t)) {
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

Token Parser::consume(TokenType type, const std::string& message)
{
    if (check(type)) return advance();
    throw error(peek(), message);
}

// ─────────────────────────────────────────────────────────────────────────────
// Error recovery: skip tokens until a likely statement boundary.
// ─────────────────────────────────────────────────────────────────────────────
void Parser::synchronize()
{
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;
        switch (peek().type) {
            case TokenType::CLASS:
            case TokenType::FUNC:
            case TokenType::LET:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::RETURN:
                return;
            default:
                break;
        }
        advance();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// AST node constructors: wrap raw data into shared‐ptr AST nodes.
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
// Helper to report parse errors via Flint::error, then throw.
// ─────────────────────────────────────────────────────────────────────────────
Parser::ParseError Parser::error(const Token& token, const std::string& message)
{
    Flint::error(token, message);
    return ParseError(message);
}

// ─────────────────────────────────────────────────────────────────────────────
// Peek and previous tokens; isAtEnd checks for EOF sentinel.
// ─────────────────────────────────────────────────────────────────────────────
bool Parser::isAtEnd() const
{
    return peek().type == TokenType::END_OF_FILE;
}

Token Parser::peek() const
{
    return tokens.at(current);
}

Token Parser::previous() const
{
    return tokens.at(current - 1);
}