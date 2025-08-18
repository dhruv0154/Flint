#pragma once
#include <algorithm>
#include <cstdlib>
#include "common.h"

class Memory {
public:
    static inline int growCapacity(int capacity) { 
        return (capacity < 8) ? 8 : capacity * 2; 
    }

    template <typename T>
    static T* growArray(T* pointer, size_t oldCount, size_t newCount) {
        // Allocate new memory with constructors
        T* result = new T[newCount];

        // Copy over existing elements
        for (size_t i = 0; i < oldCount && i < newCount; i++) {
            result[i] = pointer[i];
        }

        // Free old memory
        delete[] pointer;

        return result;
    }

    template <typename T>
    static void freeArray(T* pointer) {
        delete[] pointer;
    }
};