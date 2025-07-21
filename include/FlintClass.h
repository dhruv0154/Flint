#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "FlintCallable.h"

class FlintInstance;
class FlintFunction;

class FlintClass : public FlintCallable, public std::enable_shared_from_this<FlintClass>
{
private:
    const std::string name;
    mutable std::unordered_map<std::string, std::shared_ptr<FlintFunction>> methods;
public:
    std::string toString() const override { return name; }

    LiteralValue call(Interpreter &interpreter, 
        const std::vector<LiteralValue> &args, const Token &paren) override;

    std::shared_ptr<FlintFunction> findMethod(std::string name) const;
    int arity() const override;
    FlintClass(std::string name,
            std::unordered_map<std::string, 
                std::shared_ptr<FlintFunction>> methods) : 
                    name(name), methods(methods) {}
};