#pragma once
#include <cstddef>

template<class T>
class UniquePtr {
private:
    T* ptr;

public:
    UniquePtr() : ptr(nullptr) {}
    UniquePtr(T* p) : ptr(p) {}

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    UniquePtr(UniquePtr&& other) : ptr(other.ptr) {
        other.ptr = nullptr;
    }

    UniquePtr& operator=(UniquePtr&& other) {
        if (this != &other) {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    ~UniquePtr() { delete ptr; }

    T& operator*() { return *ptr; }
    T* operator->() { return ptr; }

    const T& operator*() const { return *ptr; }
    const T* operator->() const { return ptr; }

    operator bool() const { return ptr != nullptr; }

    T* Release() {
        T* tmp = ptr;
        ptr = nullptr;
        return tmp;
    }

    void Reset(T* p = nullptr) {
        if (ptr != p) {
            delete ptr;
            ptr = p;
        }
    }
};

template<class T>
class UniquePtr<T[]> {
private:
    T* ptr;

public:
    UniquePtr() : ptr(nullptr) {}
    UniquePtr(T* p) : ptr(p) {}

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    UniquePtr(UniquePtr&& other) : ptr(other.ptr) {
        other.ptr = nullptr;
    }

    UniquePtr& operator=(UniquePtr&& other) {
        if (this != &other) {
            delete[] ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    ~UniquePtr() { delete[] ptr; }

    T& operator[](size_t index) { return ptr[index]; }
    const T& operator[](size_t index) const { return ptr[index]; }

    operator bool() const { return ptr != nullptr; }
};