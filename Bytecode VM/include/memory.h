#pragma once

#include <cstdlib>
#include "common.h"

class Memory
{
private:
    template <typename T>
    static T* reallocate(T* pointer, size_t oldSize, size_t newSize) {
        if (newSize == 0) {
            std::free(pointer);
            return nullptr;
        }
        T* result = static_cast<T*>(std::realloc(pointer, newSize));
        if(result == nullptr) exit(1);
        return result;
    }

public:
    static inline int growCapacity(int capacity) { 
        return (capacity < 8) ? 8 : capacity * 2; 
    }
    
    template <typename T>
    static T* growArray(T* pointer, size_t oldCount, size_t newCount) {
        return static_cast<T*>
            (reallocate(
                pointer, sizeof(T) * oldCount, 
                sizeof(T) * newCount));
    }

    template <typename T>
    static T* freeArray(T* pointer, size_t oldCount) {
        return static_cast<T*>
            (reallocate(pointer, sizeof(T) * oldCount, 0));
    }

};