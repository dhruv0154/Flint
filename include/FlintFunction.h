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
public:
    FlintFunction(std::shared_ptr<FunctionStmt> declaration, std::shared_ptr<Environment> closure) 
    : declaration(std::move(declaration)), closure(closure) {}

    LiteralValue call(Interpreter &interpreter, 
        const std::vector<LiteralValue> &args, const Token &paren) override
    {
        std::shared_ptr<Environment> environment = std::make_shared<Environment>(closure);

        for(size_t i = 0; i < declaration -> params.size(); i++)
        {
            environment -> define(declaration -> params.at(i).lexeme, args.at(i));
        }
        try
        {
            interpreter.executeBlock(declaration -> body, environment);
        }
        catch(const ReturnException &e)
        {
            return e.val;
        }

        return nullptr;
    }

    int arity() const override { return declaration -> params.size(); }

    std::string toString() const override 
    {
        if(declaration -> name.has_value())
            return "<fn " + declaration -> name -> lexeme + ">";
        else
            return "<lambda>";
    }
};