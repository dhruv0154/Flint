#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  ExpressionNode System (AST Nodes for Expressions)
// ─────────────────────────────────────────────────────────────────────────────
//  Defines every kind of expression in Flint.  Parsers build these nodes,
//  and the Interpreter walks them to evaluate code.
// ─────────────────────────────────────────────────────────────────────────────

#include <memory>
#include <vector>
#include <string>
#include <variant>
#include "Flint/Scanner/Token.h"      // Token type for operators, identifiers, literals
#include "Flint/Parser/Value.h"      // LiteralValue variant holding runtime values

// ─────────────────────────────────────────────────────────────
//  Forward declarations of all statement types
// ─────────────────────────────────────────────────────────────
struct ExpressionStmt;
struct FunctionStmt;
struct IfStmt;
struct WhileStmt;
struct ReturnStmt;
struct BreakStmt;
struct ContinueStmt;
struct TryCatchContinueStmt;
struct LetStmt;
struct BlockStmt;
struct ClassStmt;

// ─────────────────────────────────────────────────────────────
//  Statement
// ─────────────────────────────────────────────────────────────
//  Variant over each statement struct.  Produced by the Parser,
//  consumed by the Interpreter (via std::visit).
using Statement = std::variant<
    ExpressionStmt,
    FunctionStmt, 
    WhileStmt,
    ReturnStmt,
    BreakStmt,
    ContinueStmt,
    TryCatchContinueStmt,
    IfStmt, 
    LetStmt, 
    BlockStmt,
    ClassStmt
>;

// ─────────────────────────────────────────────────────────────
//  Forward declarations of all expression structs
// ─────────────────────────────────────────────────────────────
struct Binary;
struct Logical;
struct Call;
struct Unary;
struct Literal;
struct Grouping;
struct Conditional;
struct Variable;
struct Assignment;
struct Lambda;
struct Get;
struct Set;
struct This;
struct Super;
struct Array;
struct GetIndex;
struct SetIndex;

// ─────────────────────────────────────────────────────────────
//  ExpressionNode
// ─────────────────────────────────────────────────────────────
//  Variant over each expression type.  An ExprPtr points to one
//  of these by wrapping in shared_ptr for polymorphic storage.
using ExpressionNode = std::variant<
    Binary,
    Call,
    Logical,
    Unary,
    Literal,
    Grouping,
    Conditional,
    Variable,
    Assignment,
    Lambda,
    Get,
    Set,
    This,
    Super,
    Array,
    GetIndex,
    SetIndex
>;

// ─────────────────────────────────────────────────────────────
//  ExprPtr
// ─────────────────────────────────────────────────────────────
//  Shared pointer to an ExpressionNode, for nesting and ownership.
using ExprPtr = std::shared_ptr<ExpressionNode>;

// ─────────────────────────────────────────────────────────────
//  Binary: left op right
// ─────────────────────────────────────────────────────────────
struct Binary {
    ExprPtr left;   // left operand
    Token    op;    // operator token (e.g. '+', '==')
    ExprPtr right;  // right operand

    Binary(ExprPtr left, Token op, ExprPtr right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
};

// ─────────────────────────────────────────────────────────────
//  Logical: left (and/or) right
// ─────────────────────────────────────────────────────────────
struct Logical {
    ExprPtr left;   // first operand
    Token    op;    // 'and' or 'or'
    ExprPtr right;  // second operand

    Logical(ExprPtr left, Token op, ExprPtr right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
};

// ─────────────────────────────────────────────────────────────
//  Conditional: condition ? thenExpr : elseExpr
// ─────────────────────────────────────────────────────────────
struct Conditional {
    ExprPtr condition;  // expression before '?'
    ExprPtr left;       // if-true branch
    ExprPtr right;      // if-false branch

    Conditional(ExprPtr condition, ExprPtr left, ExprPtr right)
        : condition(std::move(condition))
        , left(std::move(left))
        , right(std::move(right)) {}
};

// ─────────────────────────────────────────────────────────────
//  Call: function or class constructor invocation
// ─────────────────────────────────────────────────────────────
struct Call {
    ExprPtr               callee;     // expression evaluating to callable
    Token                 paren;      // closing parenthesis token (for errors)
    std::vector<ExprPtr>  arguments;  // zero or more argument expressions

    Call(ExprPtr callee, Token paren, std::vector<ExprPtr> arguments)
        : callee(std::move(callee)), paren(paren), arguments(std::move(arguments)) {}
};

// ─────────────────────────────────────────────────────────────
//  Unary: op right
// ─────────────────────────────────────────────────────────────
struct Unary {
    Token    op;     // operator token (e.g. '!', '-')
    ExprPtr  right;  // operand expression

    Unary(Token op, ExprPtr right)
        : op(std::move(op)), right(std::move(right)) {}
};

// ─────────────────────────────────────────────────────────────
//  Literal: number, bool, or nil
// ─────────────────────────────────────────────────────────────
struct Literal {
    LiteralValue value;  // holds actual runtime value

    Literal(LiteralValue value)
        : value(std::move(value)) {}
};

// ─────────────────────────────────────────────────────────────
//  Grouping: '(' expression ')'
// ─────────────────────────────────────────────────────────────
struct Grouping {
    ExprPtr expression;  // inner expression

    Grouping(ExprPtr expression)
        : expression(std::move(expression)) {}
};

// ─────────────────────────────────────────────────────────────
//  Variable: usage of identifier
// ─────────────────────────────────────────────────────────────
struct Variable {
    Token name;  // identifier token

    Variable(Token name)
        : name(std::move(name)) {}
};

// ─────────────────────────────────────────────────────────────
//  Assignment: name = value
// ─────────────────────────────────────────────────────────────
struct Assignment {
    Token   name;   // variable name token
    ExprPtr value;  // expression to assign

    Assignment(Token name, ExprPtr value)
        : name(std::move(name)), value(std::move(value)) {}
};

// ─────────────────────────────────────────────────────────────
//  Lambda: anonymous function literal
// ─────────────────────────────────────────────────────────────
struct Lambda {
    std::shared_ptr<FunctionStmt> function;  // holds parameters and body

    Lambda(std::shared_ptr<FunctionStmt> function)
        : function(std::move(function)) {}
};

// ─────────────────────────────────────────────────────────────
//  Get: property access (object.name)
// ─────────────────────────────────────────────────────────────
struct Get {
    ExprPtr object;  // expression evaluating to an instance
    Token   name;    // property name token

    Get(ExprPtr object, Token name)
        : object(std::move(object)), name(std::move(name)) {}
};

// ─────────────────────────────────────────────────────────────
//  Set: property assignment (object.name = value)
// ─────────────────────────────────────────────────────────────
struct Set {
    ExprPtr object;  // target instance expression
    Token   name;    // property name token
    ExprPtr value;   // value expression

    Set(ExprPtr object, Token name, ExprPtr value)
        : object(std::move(object))
        , name(std::move(name))
        , value(std::move(value)) {}
};

// ─────────────────────────────────────────────────────────────
//  This: usage of 'this' keyword inside class methods
// ─────────────────────────────────────────────────────────────
struct This {
    Token keyword;  // 'this' token, used for errors and binding

    This(Token keyword)
        : keyword(std::move(keyword)) {}
};

// ─────────────────────────────────────────────────────────────
//  Super: usage of 'super' keyword to call superClass methods
// ─────────────────────────────────────────────────────────────
struct Super {
    Token keyword;
    Token method;

    Super(Token keyword, Token method) : keyword(keyword), method(method) {}
};

struct Array {
    std::vector<ExprPtr> elements;

    Array(std::vector<ExprPtr> elements) : elements(std::move(elements)) {}
};

struct GetIndex {
    ExprPtr array, index;
    Token bracket;

    GetIndex(ExprPtr array, ExprPtr index, Token bracket) : array(std::move(array)), 
    index(std::move(index)), bracket(bracket) {}
};

struct SetIndex {
    ExprPtr array, index, value;
    Token bracket;

    SetIndex(ExprPtr array, ExprPtr index, ExprPtr value, Token bracket) : array(std::move(array)), 
    index(std::move(index)), value(std::move(value)), bracket(bracket) {}
};