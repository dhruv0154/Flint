#pragma once

#include <string>
#include <unordered_map>
#include "Flint/Parser/Value.h"
#include "Flint/Scanner/Token.h"
#include "Flint/Callables/Functions/BuiltInFunction.h"

class FlintString : public std::enable_shared_from_this<FlintString> {

private:
    std::unordered_map<std::string, std::shared_ptr<BuiltinFunction>> builtInFunctions;

public:
    // Underlying storage
    std::string value;
    LiteralValue getInBuiltFunction(Token name);

    // Construct with some value
    FlintString(std::string value);
};