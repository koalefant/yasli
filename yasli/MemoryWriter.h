#pragma once

#include <cstddef>
#include "UtilsAPI.h"
#include "Pointers.h"

#ifdef realloc
#undef realloc
#endif

#ifndef WIN32
typedef long long __int64;
#endif

class UTILS_API MemoryWriter : public RefCounter{
public:
    MemoryWriter(std::size_t size = 128, bool reallocate = true);
    ~MemoryWriter();

    const char* c_str() { return memory_; };
    const wchar_t* w_str() { return (wchar_t*)memory_; };
    void* buffer() { return (void*)(memory_); }
    const void* buffer() const { return (const void*)(memory_); }
    std::size_t size() const{ return size_; }

    // String interface (after this calls '\0' is always written)
    MemoryWriter& operator<<(int value);
    MemoryWriter& operator<<(long value);
    MemoryWriter& operator<<(unsigned long value);
    MemoryWriter& operator<<(unsigned int value);
    MemoryWriter& operator<<(__int64 value);
    MemoryWriter& operator<<(float value);
	MemoryWriter& operator<<(double value);
    MemoryWriter& operator<<(signed char value);
    MemoryWriter& operator<<(unsigned char value);
    MemoryWriter& operator<<(char value);
    MemoryWriter& operator<<(const char* value);
    MemoryWriter& operator<<(const wchar_t* value);

    // Binary interface (does not writes trailing '\0')
    template<class T>
    void write(const T& value){
        write(reinterpret_cast<const T*>(&value), sizeof(value));
    }
    void write(char c);
    void write(const char* str);
    void write(const void* data, std::size_t size);

    std::size_t position() const{ return position_ - memory_; }
    void setPosition(std::size_t pos);
private:
    void alloc(std::size_t initialSize);
    void realloc(std::size_t newSize);

    std::size_t size_;
    char* position_;
    char* memory_;
    bool reallocate_;
};
