#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "Flint/Parser/Value.h"
#include "Flint/Scanner/Token.h"
#include "Flint/Interpreter/Interpreter.h"

class FlintClass;

// FlintInstance represents an instance of a class at runtime.
// It stores fields and handles method/property lookups.
class FlintInstance : public std::enable_shared_from_this<FlintInstance>
{
private:
    // Pointer to the class this instance was created from.
    std::shared_ptr<FlintClass> klass;

    // Stores instance-specific fields (properties and values).
    std::unordered_map<std::string, LiteralValue> fields;

public:
    // Default constructor (rarely used; mostly for safety or placeholder).
    FlintInstance() = default;

    // Constructor to create an instance from a given class.
    explicit FlintInstance(std::shared_ptr<FlintClass> klass) : klass(klass) {}

    // Called when accessing a property or method on the instance.
    // If the name is a field, it returns it. Otherwise, it tries to return a bound method.
    virtual LiteralValue get(Token name, Interpreter& interpreter);

    // Called when assigning a value to a field.
    // If the field doesn't exist, it's created dynamically.
    void set(Token name, LiteralValue object);

    // Returns a string representation of the instance.
    std::string toString() const;
};