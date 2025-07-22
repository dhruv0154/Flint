#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "FlintCallable.h"
#include "FlintInstance.h"

class FlintFunction;

class FlintClass : public FlintCallable, public FlintInstance
{
private:
    const std::string name;
    mutable std::unordered_map<std::string, std::shared_ptr<FlintFunction>> instanceMethods;
    mutable std::unordered_map<std::string, std::shared_ptr<FlintFunction>> classMethods;
public:
    std::string toString() const override { return name; }
    LiteralValue get(Token name, Interpreter& interpreter) override;

    LiteralValue call(Interpreter &interpreter, 
        const std::vector<LiteralValue> &args, const Token &paren) override;

    std::shared_ptr<FlintFunction> findMethod(std::string name) const;
    int arity() const override;
    FlintClass(std::string name,
            std::unordered_map<std::string, 
                std::shared_ptr<FlintFunction>> instanceMethods,
            std::unordered_map<std::string, 
                std::shared_ptr<FlintFunction>> classMethods) : 
                    name(name), instanceMethods(instanceMethods),
                     classMethods(classMethods) {}
};