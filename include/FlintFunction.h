#include "Stmt.h"
#include "Interpreter/Interpreter.h"
#include "FlintCallable.h"
#include "Flint/Environment.h"
#include "ReturnException.h"

// FlintFunction represents a user-defined function in the language.
// It implements FlintCallable, meaning it can be "called" like a function.
class FlintFunction : public FlintCallable
{
private:
    // The environment where the function was defined; used to capture closures.
    std::shared_ptr<Environment> closure;

    // Indicates whether this function is an initializer (i.e., a constructor).
    bool isInitializer;

public:
    // AST node representing the function declaration.
    std::shared_ptr<FunctionStmt> declaration;

    // Constructor initializes the function with its declaration, closure, and initializer flag.
    FlintFunction(std::shared_ptr<FunctionStmt> declaration, 
                  std::shared_ptr<Environment> closure,
                  bool isInitializer) 
        : declaration(std::move(declaration)), closure(closure), isInitializer(isInitializer) {}

    // This function is called when the function is invoked in the source code.
    // Example: myFunc(1, 2); -> triggers call() with 1 and 2 as args.
    LiteralValue call(Interpreter &interpreter, 
                      const std::vector<LiteralValue> &args, const Token &paren) override;

    // Returns the number of parameters the function expects.
    int arity() const override;

    // Returns a string representation of the function (usually the name or "<fn>")
    std::string toString() const override;

    // Binds 'this' to a given instance in methods.
    // Used to bind methods to class instances so `this` works correctly.
    LiteralValue bind(LiteralValue instance);
};