#pragma once

#include <array>
#include "token.h"
#include "compiler.h"
#include "value.h"
#include "scanner.h"

enum Precedence {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_CALL,        // . ()
  PREC_PRIMARY
};

class Parser;
class Compiler;


using ParseFn = void (*)(Parser&);

struct ParseRule{
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
};

class Parser
{
private:    
    void errorAtCurrent(const std::string_view &msg);
    void errorAt(Token* token, const std::string_view &msg);
    void parsePrecedence(Precedence precedence);

    const ParseRule& getRule(TokenType type);
   
public:
    static void number(Parser& p);
    static void grouping(Parser& p);
    static void unary(Parser& p);
    static void binary(Parser& p);
    Scanner scanner;
    Compiler* compiler;
    Parser(Scanner scanner, Compiler* compiler) : scanner(scanner), 
        compiler(compiler) {}
    Parser(Compiler* compiler) : compiler(compiler) {}
    ~Parser() = default;
    bool hadError = false;
    bool panicMode = false;
    Token current;
    Token previous;
    void error(const std::string_view &msg);
    void advance();
    void consume(TokenType type, const std::string_view &msg);
    void expression();
};

// Total number of token types (adjust if you add more).
constexpr size_t TOKEN_COUNT = static_cast<size_t>(TokenType::END_OF_FILE) + 1;  

// Rules table (constexpr + std::array for safety).
constexpr std::array<ParseRule, TOKEN_COUNT> rules = {{
    /* LEFT_PAREN        */ {Parser::grouping, nullptr, PREC_NONE},
    /* RIGHT_PAREN       */ {nullptr, nullptr, PREC_NONE},
    /* LEFT_BRACE        */ {nullptr, nullptr, PREC_NONE},
    /* RIGHT_BRACE       */ {nullptr, nullptr, PREC_NONE},
    /* LEFT_BRACKET      */ {nullptr, nullptr, PREC_NONE},
    /* RIGHT_BRACKET     */ {nullptr, nullptr, PREC_NONE},
    /* COMMA             */ {nullptr, nullptr, PREC_NONE},
    /* DOT               */ {nullptr, nullptr, PREC_NONE},
    /* MINUS             */ {Parser::unary, Parser::binary, PREC_TERM},
    /* PLUS              */ {nullptr, Parser::binary, PREC_TERM},
    /* MODULO            */ {nullptr, Parser::binary, PREC_FACTOR}, // Optional
    /* COLON             */ {nullptr, nullptr, PREC_NONE},
    /* QUESTION_MARK     */ {nullptr, nullptr, PREC_NONE},
    /* SEMICOLON         */ {nullptr, nullptr, PREC_NONE},
    /* SLASH             */ {nullptr, Parser::binary, PREC_FACTOR},
    /* STAR              */ {nullptr, Parser::binary, PREC_FACTOR},
    /* BANG              */ {nullptr, nullptr, PREC_NONE},
    /* BANG_EQUAL        */ {nullptr, nullptr, PREC_NONE},
    /* EQUAL             */ {nullptr, nullptr, PREC_NONE},
    /* EQUAL_EQUAL       */ {nullptr, nullptr, PREC_NONE},
    /* GREATER           */ {nullptr, nullptr, PREC_NONE},
    /* GREATER_EQUAL     */ {nullptr, nullptr, PREC_NONE},
    /* LESS              */ {nullptr, nullptr, PREC_NONE},
    /* LESS_EQUAL        */ {nullptr, nullptr, PREC_NONE},
    /* IDENTIFIER        */ {nullptr, nullptr, PREC_NONE},
    /* STRING            */ {nullptr, nullptr, PREC_NONE},
    /* NUMBER            */ {Parser::number, nullptr, PREC_NONE},
    /* PRINT             */ {nullptr, nullptr, PREC_NONE},
    /* AND               */ {nullptr, nullptr, PREC_NONE},
    /* CLASS             */ {nullptr, nullptr, PREC_NONE},
    /* ELSE              */ {nullptr, nullptr, PREC_NONE},
    /* FALSE             */ {nullptr, nullptr, PREC_NONE},
    /* FUNC              */ {nullptr, nullptr, PREC_NONE},
    /* FOR               */ {nullptr, nullptr, PREC_NONE},
    /* IF                */ {nullptr, nullptr, PREC_NONE},
    /* NOTHING           */ {nullptr, nullptr, PREC_NONE},
    /* OR                */ {nullptr, nullptr, PREC_NONE},
    /* RETURN            */ {nullptr, nullptr, PREC_NONE},
    /* SUPER             */ {nullptr, nullptr, PREC_NONE},
    /* THIS              */ {nullptr, nullptr, PREC_NONE},
    /* TRUE              */ {nullptr, nullptr, PREC_NONE},
    /* LET               */ {nullptr, nullptr, PREC_NONE},
    /* WHILE             */ {nullptr, nullptr, PREC_NONE},
    /* ERROR             */ {nullptr, nullptr, PREC_NONE},
    /* END_OF_FILE       */ {nullptr, nullptr, PREC_NONE},
}};