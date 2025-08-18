#include <iostream>
#include "value.h"

void ValueArray::writeValueArray(Value val)
{
    if(capacity < count + 1)
    {
        int oldCapacity = capacity;
        capacity = Memory::growCapacity(oldCapacity);
        values = Memory::growArray(values, oldCapacity, capacity);
    }
    values[count++] = val;
}

void ValueArray::printValue(Value val)
{
    std::cout << val;
}