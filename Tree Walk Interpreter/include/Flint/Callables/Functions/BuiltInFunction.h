#pragma once

#include <functional>
#include "Flint/Parser/Value.h"
#include "Flint/Callables/FlintCallable.h"

class BuiltinFunction : public FlintCallable {
public:
    using BuiltinFn = std::function<LiteralValue(Interpreter&, const std::vector<LiteralValue>&, const Token&)>;
    
    BuiltinFunction(BuiltinFn fn, int arity)
        : fn_(std::move(fn)), arity_(arity) {}

    int arity() const override { return arity_; }

    LiteralValue call(Interpreter& interpreter, const std::vector<LiteralValue>& args, const Token& token) override {
        return fn_(interpreter, args, token);
    }

private:
    BuiltinFn fn_;
    int arity_;
};