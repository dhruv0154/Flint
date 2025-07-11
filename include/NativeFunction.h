#pragma once

#include "FlintCallable.h"
#include <functional>
#include <string>

class NativeFunction : public FlintCallable 
{
public:
    using NativeFn = std::function<LiteralValue
    (const std::vector<LiteralValue>&, 
        const Token &paren)>;

    NativeFunction(int arity, NativeFn fn, std::string name = "") 
        : arity_(arity), fn_(std::move(fn)), 
        name_(std::move(name)) {}

    int arity() const override {
        return arity_;
    }

    LiteralValue call(Interpreter&, 
        const std::vector<LiteralValue>& args, const Token &paren) override {
        return fn_(args, paren);
    }

    std::string toString() const override {
        return "<native fn: " + name_ + ">";
    }

private:
    int arity_;
    NativeFn fn_;
    std::string name_;
};