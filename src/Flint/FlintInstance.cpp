#include "FlintInstance.h"
#include "FlintClass.h"
#include "RuntimeError.h"
#include "FlintFunction.h"

std::string FlintInstance::toString() const {
    return klass->toString() + " instance";
}

LiteralValue FlintInstance::get(Token name)
{
    if(fields.count(name.lexeme)) return fields[name.lexeme];
    LiteralValue method = klass -> findMethod(name.lexeme);
    if(!std::holds_alternative<nullptr_t>(method))
    {
        auto callable = std::get<std::shared_ptr<FlintCallable>>(method);
        auto fn = std::dynamic_pointer_cast<FlintFunction>(callable);
        return fn -> bind(shared_from_this());
    }
    throw RuntimeError(name, "Undefined property '" + name.lexeme + "'." );
}

void FlintInstance::set(Token name, LiteralValue object)
{
    fields[name.lexeme] = object;
}