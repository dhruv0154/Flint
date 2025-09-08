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

template <typename T>
class DynamicArray {
private:
    size_t capacity = 0;
    size_t count = 0;
    T* data = nullptr;

public:
    ~DynamicArray() { delete[] data; }
    DynamicArray() = default;
    DynamicArray(const DynamicArray&) = delete;
    DynamicArray& operator=(const DynamicArray&) = delete;

    void push_back(const T& value) {
        if (count + 1 > capacity) {
            size_t newCapacity = Memory::growCapacity(capacity);
            data = Memory::growArray(data, count, newCapacity);
            capacity = newCapacity;
        }
        data[count++] = value;
    }

    T pop(){ 
        count--;
        return data[count];
    }
    T* getData() { return data; }
    const T* getData() const { return data; }

    T& operator[](size_t i) { return data[i]; }
    const T& operator[](size_t i) const { return data[i]; }

    size_t size() const { return count; }
};