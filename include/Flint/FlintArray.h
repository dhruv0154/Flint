#pragma once

#include <vector>
#include "Flint/Parser/Value.h"
#include "Flint/Interpreter/Interpreter.h"

class FlintArray {
public:
    // Underlying storage
    std::vector<LiteralValue> elements;

    // Construct an empty array
    FlintArray() = default;

    std::string toString()  {
       std::string out = "[";
        for (size_t i = 0; i < elements.size(); ++i) {
            out += Interpreter::stringify(elements[i]);
            if (i + 1 < elements.size()) out += ", ";
        }
        out += "]";
        return out;
    }

    // Construct with initial elements
    explicit FlintArray(std::vector<LiteralValue> elems)
        : elements(std::move(elems)) {}
};