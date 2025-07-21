#include "FlintClass.h"
#include "FlintFunction.h"
#include "FlintInstance.h"

LiteralValue FlintClass::call(Interpreter &interpreter, 
        const std::vector<LiteralValue> &args, const Token &paren)
{
    std::shared_ptr<FlintClass> sharedThis = std::static_pointer_cast<FlintClass>(shared_from_this());
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
    if(methods.count(name)) return methods[name];
    return nullptr;
}

int FlintClass::arity() const
{
    std::shared_ptr<FlintFunction> initializer = findMethod("init");
    if(initializer) return initializer -> arity();
    return 0;
}