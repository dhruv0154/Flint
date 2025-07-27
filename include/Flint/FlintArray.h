#pragma once

#include <vector>
#include "Flint/Parser/Value.h"

class FlintArray {
public:
    // Underlying storage
    std::vector<LiteralValue> elements;

    // Construct an empty array
    FlintArray() = default;

    // Construct with initial elements
    explicit FlintArray(std::vector<LiteralValue> elems)
        : elements(std::move(elems)) {}
};