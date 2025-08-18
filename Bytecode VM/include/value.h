#pragma once

#include "common.h"
#include "memory.h"

using Value = double;

class ValueArray
{
private:
    int capacity;
    int count;
    Value* values;
public:
    ValueArray() : count(0), capacity(0), values(nullptr) {}
    ValueArray(const ValueArray&) = delete;
    ValueArray& operator=(const ValueArray&) = delete;
    ValueArray(ValueArray&&) = delete;
    ValueArray& operator=(ValueArray&&) = delete;
    
    void writeValueArray(Value val);
    void printValue(Value val);
    int getCount() { return count; }
    int getCapacity() { return capacity; }
    Value* getValues() { return values; }

    ~ValueArray() { Memory::freeArray(values); }
};