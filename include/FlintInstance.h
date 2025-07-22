#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "Scanner/Value.h"
#include "Scanner/Token.h"
#include "Interpreter/Interpreter.h"

class FlintClass;

class FlintInstance : public std::enable_shared_from_this<FlintInstance>
{
private:
    std::shared_ptr<FlintClass> klass;
    std::unordered_map<std::string, LiteralValue> fields;
public:
    FlintInstance() = default;      
    explicit FlintInstance(std::shared_ptr<FlintClass> klass) : klass(klass) {}
    virtual LiteralValue get(Token name, Interpreter& interpreter);
    void set(Token name, LiteralValue object);
    std::string toString() const;
};