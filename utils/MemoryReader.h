#pragma once

#include <cstddef>
#include "UtilsAPI.h"

class UTILS_API MemoryReader{
public:

    MemoryReader();
    explicit MemoryReader(const char* fileName);
    MemoryReader(const void* memory, std::size_t size, bool ownAndFree = false);
    ~MemoryReader();

    void setPosition(const char* position);
    const char* position(){ return position_; }

    template<class T>
    void read(T& value){
        read(reinterpret_cast<void*>(&value), sizoef(value));
    }
    void read(void* data, std::size_t size);
    bool checkedSkip(std::size_t size);
    bool checkedRead(void* data, std::size_t size);
    template<class T>
    bool checkedRead(T& t){
        return checkedRead((void*)&t, sizeof(t));
    }

    const char* buffer() const{ return memory_; }
    std::size_t size() const{ return size_; }

    const char* begin() const{ return memory_; }
    const char* end() const{ return memory_ + size_; }
private:
    std::size_t size_;
    const char* position_;
    const char* memory_;
    bool ownedMemory_;
};
