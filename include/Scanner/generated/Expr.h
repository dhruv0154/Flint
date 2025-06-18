#pragma once

#include <memory>
#include <vector>
#include <string>
#include <any>
#include "Scanner/Token.h"

class Expr
{
public:
    virtual ~Expr() = default;
};

class Binary : public Expr
{
  public:
    Binary(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right) : left(left), op(op), right(right) {}

  std::shared_ptr<Expr> left;
  Token op;
  std::shared_ptr<Expr> right;
};

class Grouping : public Expr
{
  public:
    Grouping(std::shared_ptr<Expr> expression) : expression(expression) {}

  std::shared_ptr<Expr> expression;
};

class Literal : public Expr
{
  public:
    Literal(std::any value) : value(value) {}

  std::any value;
};

class Unary : public Expr
{
  public:
    Unary(Token op, std::shared_ptr<Expr> right) : op(op), right(right) {}

  Token op;
  std::shared_ptr<Expr> right;
};

