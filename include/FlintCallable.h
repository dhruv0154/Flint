#pragma once
#include <vector>
#include <string>
#include "Scanner/Token.h"
#include "Scanner/Value.h"

class Interpreter;

class FlintCallable {
public:
    virtual int arity() const = 0;
    virtual LiteralValue call(Interpreter &interpreter, 
        const std::vector<LiteralValue> &arguments,
        const Token &paren) = 0;
    virtual std::string toString() const { return "<fn>"; }

    virtual ~FlintCallable() = default;
};
