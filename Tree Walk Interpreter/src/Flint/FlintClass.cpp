#include "Flint/Callables/Classes/FlintClass.h"
#include "Flint/Callables/Functions/FlintFunction.h"
#include "Flint/Callables/Classes/FlintInstance.h"
#include "Flint/Exceptions/RuntimeError.h"

// ─────────────────────────────────────────────────────────────
// Calls the class like a function (for instantiation).
// This is how class constructors are invoked in Flint.
//
// 1. Creates a new instance of the class.
// 2. Looks for an "init" method (constructor), if present.
// 3. If found, binds it to the instance and calls it with args.
// 4. Returns the created instance as the result of the call.
// ─────────────────────────────────────────────────────────────
LiteralValue FlintClass::call(Interpreter &interpreter, 
        const std::vector<LiteralValue> &args, const Token &paren)
{
    // Create a shared_ptr to this class object (as a callable class)
    auto instPtr = FlintInstance::shared_from_this();

    // Cast the base instance to the derived FlintClass
    auto sharedThis = std::static_pointer_cast<FlintClass>(instPtr);

    // Create the actual object (instance of the class)
    std::shared_ptr<FlintInstance> instance = std::make_shared<FlintInstance>(sharedThis);

    // Look for an "init" method (constructor)
    std::shared_ptr<FlintFunction> initializer = findMethod("init");

    if (initializer != nullptr) 
    {
        // Bind the init method to this instance and call it
        std::get<std::shared_ptr<FlintCallable>>(initializer->bind(instance))->call(interpreter, args, paren);
    }

    // Return the created instance as the result of the "call"
    return instance;
}

// ─────────────────────────────────────────────────────────────
// Looks up a method by name in the class's instanceMethods map.
// Used to retrieve methods like "init", or user-defined ones.
// Returns nullptr if not found.
// ─────────────────────────────────────────────────────────────
std::shared_ptr<FlintFunction> FlintClass::findMethod(std::string name) const
{
    if (instanceMethods.count(name)) {
        return instanceMethods.at(name);
    }

    if (superClass)
    {
        return superClass -> findMethod(name);
    }
    return nullptr;
}

// ─────────────────────────────────────────────────────────────
// Gets a static/class-level method or property.
// Only allows access to class-level (not instance-level) members.
//
// Throws RuntimeError if the property is not found.
// ─────────────────────────────────────────────────────────────
LiteralValue FlintClass::get(Token name, Interpreter& interpreter)
{
    if (classMethods.find(name.lexeme) != classMethods.end()) {
        return classMethods[name.lexeme];
    }

    throw RuntimeError(name, "Undefined static property '" + name.lexeme + "'.");
}

// ─────────────────────────────────────────────────────────────
// Returns the number of parameters the "init" method (if present) takes.
// This is needed because the class is also a callable and must report arity.
// If no init method, returns 0.
// ─────────────────────────────────────────────────────────────
int FlintClass::arity() const
{
    std::shared_ptr<FlintFunction> initializer = findMethod("init");
    if (initializer) return initializer->arity();
    return 0;
}