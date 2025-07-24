#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "FlintCallable.h"
#include "FlintInstance.h"

// Forward declaration to avoid circular dependency
class FlintFunction;

// FlintClass represents user-defined classes in Flint.
// It is both callable (acts like a constructor) and an instance (supports method lookup).
class FlintClass : public FlintCallable, public FlintInstance
{
private:
    // Name of the class (used for display and debugging)
    const std::string name;

    // Methods that instances of this class can call
    mutable std::unordered_map<std::string, std::shared_ptr<FlintFunction>> instanceMethods;

    // Static methods defined on the class itself
    mutable std::unordered_map<std::string, std::shared_ptr<FlintFunction>> classMethods;

public:
    // Returns a string representation of the class (e.g., the class name)
    std::string toString() const override { return name; }

    // Handles property/method access on the class itself (like static methods)
    LiteralValue get(Token name, Interpreter& interpreter) override;

    // When the class is called (e.g., Circle(4)), this constructs a new instance.
    LiteralValue call(Interpreter &interpreter, 
                      const std::vector<LiteralValue> &args, 
                      const Token &paren) override;

    // Looks up an instance method by name
    std::shared_ptr<FlintFunction> findMethod(std::string name) const;

    // Returns the number of parameters expected by the class's constructor
    int arity() const override;

    // Constructor to initialize the class with its name, instance methods, and class (static) methods
    FlintClass(std::string name,
               std::unordered_map<std::string, std::shared_ptr<FlintFunction>> instanceMethods,
               std::unordered_map<std::string, std::shared_ptr<FlintFunction>> classMethods) : 
        name(name), instanceMethods(instanceMethods), classMethods(classMethods) {}
};
