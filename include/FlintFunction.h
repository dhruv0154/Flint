#include "Stmt.h"
#include "Interpreter/Interpreter.h"
#include "FlintCallable.h"
#include "Flint/Environment.h"
#include "ReturnException.h"

class FlintFunction : public FlintCallable
{
private:
    std::shared_ptr<FunctionStmt> declaration;
    std::shared_ptr<Environment> closure;
    bool isInitializer;
public:
    FlintFunction(std::shared_ptr<FunctionStmt> declaration, 
            std::shared_ptr<Environment> closure,
            bool isInitializer) 
    : declaration(std::move(declaration)), closure(closure), isInitializer(isInitializer) {}

    LiteralValue call(Interpreter &interpreter, 
        const std::vector<LiteralValue> &args, const Token &paren) override;

    int arity() const override;

    std::string toString() const override;

    LiteralValue bind(LiteralValue instance);
};