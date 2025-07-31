#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include "Flint/Parser/Value.h"
#include "Flint/Callables/Functions/BuiltInFunction.h"
#include "Flint/Interpreter/Interpreter.h"

class FlintArray {
private:
    std::unordered_map<std::string, std::shared_ptr<BuiltinFunction>> builtInFunctions;

public:
    // Underlying storage
    std::vector<LiteralValue> elements;

    std::string toString()  {
       std::string out = "[";
        for (size_t i = 0; i < elements.size(); ++i) {
            out += Interpreter::stringify(elements[i]);
            if (i + 1 < elements.size()) out += ", ";
        }
        out += "]";
        return out;
    }

    LiteralValue getInBuiltFunction(Token name);

    FlintArray(std::vector<LiteralValue> elems);
};