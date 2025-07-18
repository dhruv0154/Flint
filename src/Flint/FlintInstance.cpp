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
    if(!std::holds_alternative<nullptr_t>(method)) return method;
    throw RuntimeError(name, "Undefined property '" + name.lexeme + "'." );
}

void FlintInstance::set(Token name, LiteralValue object)
{
    fields[name.lexeme] = object;
}