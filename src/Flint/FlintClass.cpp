#include "FlintClass.h"
#include "FlintFunction.h"
#include "FlintInstance.h"

LiteralValue FlintClass::call(Interpreter &interpreter, 
        const std::vector<LiteralValue> &args, const Token &paren)
{
    std::shared_ptr<FlintClass> sharedThis = std::static_pointer_cast<FlintClass>(shared_from_this());
    std::shared_ptr<FlintInstance> instance = std::make_shared<FlintInstance>(sharedThis);
    return instance;
}

std::shared_ptr<FlintFunction> FlintClass::findMethod(std::string name)
{
    if(methods.count(name)) return methods[name];
    return nullptr;
}