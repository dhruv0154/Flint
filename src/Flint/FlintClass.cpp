#include "FlintClass.h"
#include "FlintFunction.h"
#include "FlintInstance.h"
#include "RuntimeError.h"

LiteralValue FlintClass::call(Interpreter &interpreter, 
        const std::vector<LiteralValue> &args, const Token &paren)
{
    auto instPtr = FlintInstance::shared_from_this();
    auto sharedThis = std::static_pointer_cast<FlintClass>(instPtr);
    std::shared_ptr<FlintInstance> instance = std::make_shared<FlintInstance>(sharedThis);
    std::shared_ptr<FlintFunction> intializer = findMethod("init");
    if(intializer != nullptr) 
    {
        std::get<std::shared_ptr<FlintCallable>>
            (intializer -> bind(instance)) -> call(interpreter, args, paren);
    }
    return instance;
}

std::shared_ptr<FlintFunction> FlintClass::findMethod(std::string name) const
{
    if(instanceMethods.count(name))
    {
        return instanceMethods[name];
    }
    return nullptr;
}

LiteralValue FlintClass::get(Token name, Interpreter& interpreter)
{      
    if (classMethods.find(name.lexeme) != classMethods.end()) {
        return classMethods[name.lexeme];
    }

    throw RuntimeError(name, "Undefined static property '" + name.lexeme + "'.");
}

int FlintClass::arity() const
{
    std::shared_ptr<FlintFunction> initializer = findMethod("init");
    if(initializer) return initializer -> arity();
    return 0;
}