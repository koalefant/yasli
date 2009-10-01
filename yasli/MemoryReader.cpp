#include "StdAfx.h"
#include "yasli/Assert.h"
#include "yasli/MemoryReader.h"
#include <cstdlib>
#include <iostream>
#include "Unicode.h"

namespace yasli{

MemoryReader::MemoryReader()
: size_(0)
, position_(0)
, memory_(0)
, ownedMemory_(false)
{
}

MemoryReader::MemoryReader(const char* fileName)
{
    FILE* file = ::yasli::fopen(fileName, "rb");
    ASSERT(file);
    if(file){
        fseek(file, 0, SEEK_END);
        std::size_t len = ftell(file);
        fseek(file, 0, SEEK_SET);
        memory_ = new char[len];
        std::size_t count = fread((void*)memory_, 1, len, file);
        ASSERT(count == len);
        ownedMemory_ = true;
        position_ = memory_;
        size_ = len;
    }
}

MemoryReader::MemoryReader(const void* memory, std::size_t size, bool ownAndFree)
: size_(size)
, position_((const char*)(memory))
, memory_((const char*)(memory))
, ownedMemory_(ownAndFree)
{

}

MemoryReader::~MemoryReader()
{
    if(ownedMemory_){
        std::free(const_cast<char*>(memory_));
        memory_ = 0;
        size_ = 0;
    }
}

void MemoryReader::setPosition(const char* position)
{
    position_ = position;
}

void MemoryReader::read(void* data, std::size_t size)
{
    ASSERT(memory_ && position_);
    ASSERT(position_ - memory_ + size <= size_);
    ::memcpy(data, position_, size);
    position_ += size;
}

bool MemoryReader::checkedRead(void* data, std::size_t size)
{
    if(!memory_ || !position_)
        return false;
    if(position_ - memory_ + size > size_)
        return false;

    ::memcpy(data, position_, size);
    position_ += size;
    return true;
}

bool MemoryReader::checkedSkip(std::size_t size)
{
    if(!memory_ || !position_)
        return false;
    if(position_ - memory_ + size > size_)
        return false;

    position_ += size;
    return true;
}

}
