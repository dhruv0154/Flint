#pragma once

#include <memory>
#include <vector>
#include <string>
#include <any>
#include "C:\Flint\include\Scanner\Token.h"

// Forward declarations
template <typename T> class Expr;
template <typename T> class Visitor;
template <typename T> class Binary;
template <typename T> class Grouping;
template <typename T> class Literal;
template <typename T> class Unary;

template <typename T> 
class Expr
{
    public:
        virtual T accept(Visitor<T>& visitor) = 0;
        virtual ~Expr() = default;
};

template <typename T> 
class Visitor 
{
    public:
        virtual T visitBinaryExpr(Binary<T>& expr) = 0;
        virtual T visitGroupingExpr(Grouping<T>& expr) = 0;
        virtual T visitLiteralExpr(Literal<T>& expr) = 0;
        virtual T visitUnaryExpr(Unary<T>& expr) = 0;
        virtual ~Visitor() = default;
};

template <typename T>
class Binary : public Expr<T>
{
    public:
        Binary(std::shared_ptr<Expr<T>> left, Token op, std::shared_ptr<Expr<T>> right) : left(left), op(op), right(right) {}
        T accept(Visitor<T>& visitor) override { return visitor.visitBinaryExpr(*this); }

        std::shared_ptr<Expr<T>> left;
        Token op;
        std::shared_ptr<Expr<T>> right;
};

template <typename T>
class Grouping : public Expr<T>
{
    public:
        Grouping(std::shared_ptr<Expr<T>> expression) : expression(expression) {}
        T accept(Visitor<T>& visitor) override { return visitor.visitGroupingExpr(*this); }

        std::shared_ptr<Expr<T>> expression;
};

template <typename T>
class Literal : public Expr<T>
{
    public:
        Literal(std::any value) : value(value) {}
        T accept(Visitor<T>& visitor) override { return visitor.visitLiteralExpr(*this); }

        std::any value;
};

template <typename T>
class Unary : public Expr<T>
{
    public:
        Unary(Token op, std::shared_ptr<Expr<T>> right) : op(op), right(right) {}
        T accept(Visitor<T>& visitor) override { return visitor.visitUnaryExpr(*this); }

        Token op;
        std::shared_ptr<Expr<T>> right;
};

