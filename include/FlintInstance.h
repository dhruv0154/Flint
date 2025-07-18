#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "Scanner/Value.h"
#include "Scanner/Token.h"

class FlintClass;

class FlintInstance
{
private:
    std::shared_ptr<FlintClass> klass;
    std::unordered_map<std::string, LiteralValue> fields;
public:
    FlintInstance(std::shared_ptr<FlintClass> klass) : klass(klass) {}
    LiteralValue get(Token name);
    void set(Token name, LiteralValue object);
    std::string toString() const;
};