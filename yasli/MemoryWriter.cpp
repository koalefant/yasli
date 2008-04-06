#include "StdAfx.h"
#include "yasli/Errors.h"

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <math.h>
#ifdef WIN32
# include <float.h>
# define isnan _isnan
#endif

#include "yasli/MemoryWriter.h"

MemoryWriter::MemoryWriter(std::size_t size, bool reallocate)
: size_(size)
, reallocate_(reallocate)
{
    alloc(size);
}

MemoryWriter::~MemoryWriter()
{
    position_ = 0;
    std::free(memory_);
}

void MemoryWriter::alloc(std::size_t initialSize)
{
    memory_ = (char*)std::malloc(initialSize + 1);
    position_ = memory_;
}

void MemoryWriter::realloc(std::size_t newSize)
{
    ASSERT(newSize > size_);
    std::size_t pos = position();
    memory_ = (char*)std::realloc(memory_, newSize + 1);
    ASSERT(memory_);
    position_ = memory_ + pos;
    size_ = newSize;
}

MemoryWriter& MemoryWriter::operator<<(int value)
{
    // TODO: optimize
    char buffer[12];
    sprintf(buffer, "%i", value);
    return operator<<((const char*)buffer);
}

MemoryWriter& MemoryWriter::operator<<(long value)
{
    // TODO: optimize
    char buffer[12];
    sprintf(buffer, "%i", value);
    return operator<<((const char*)buffer);
}

MemoryWriter& MemoryWriter::operator<<(unsigned long value)
{
    // TODO: optimize
    char buffer[12];
    sprintf(buffer, "%u", value);
    return operator<<((const char*)buffer);
}

MemoryWriter& MemoryWriter::operator<<(__int64 value)
{
    // TODO: optimize
    char buffer[24];
    sprintf(buffer, "%I64u", value);
    return operator<<((const char*)buffer);
}

MemoryWriter& MemoryWriter::operator<<(unsigned int value)
{
    // TODO: optimize
    char buffer[12];
    sprintf(buffer, "%u", value);
    return operator<<((const char*)buffer);
}

MemoryWriter& MemoryWriter::operator<<(char value)
{
    char buffer[12];
    sprintf(buffer, "%i", int(value));
    return operator<<((const char*)buffer);
}

MemoryWriter& MemoryWriter::operator<<(unsigned char value)
{
    char buffer[12];
    sprintf(buffer, "%i", int(value));
    return operator<<((const char*)buffer);
}

MemoryWriter& MemoryWriter::operator<<(signed char value)
{
    char buffer[12];
    sprintf(buffer, "%i", int(value));
    return operator<<((const char*)buffer);
}

MemoryWriter& MemoryWriter::operator<<(float value)
{
	ASSERT(!isnan(value));
#ifdef WIN32
	char buf[_CVTBUFSIZE];
	_gcvt_s(buf, double(value), 8);
	write(buf);
#else
    int point = 0;
    int sign = 0;
    const char* buf = fcvt(double(value), 5, &point, &sign);

	if(sign != 0)
        write("-");
    if(point <= 0){
        write("0.");
        while(point < 0){
            write("0");
            ++point;
        }
        write(buf);
        *position_ = '\0';
    }
    else{
        write(buf, point);
        write(".");
        operator<<(buf + point);
    }
#endif
    return *this;
}

MemoryWriter& MemoryWriter::operator<<(double value)
{
	ASSERT(!isnan(value));
#ifdef WIN32
	char buf[_CVTBUFSIZE];
	_gcvt_s(buf, value, 8);
	write(buf);
#else
    int point = 0;
    int sign = 0;
    const char* buf = fcvt(value, 5, &point, &sign);

    if(sign != 0)
        write("-");
    if(point <= 0){
        write("0.");
        while(point < 0){
            write("0");
            ++point;
        }
        write(buf);
        *position_ = '\0';
    }
    else{
        write(buf, point);
        write(".");
        operator<<(buf + point);
    }
#endif
    return *this;
}

MemoryWriter& MemoryWriter::operator<<(const char* value)
{
    write((void*)value, strlen(value));
    ASSERT(position() < size());
    *position_ = '\0';
    return *this;
}

MemoryWriter& MemoryWriter::operator<<(const wchar_t* value)
{
    write((void*)value, wcslen(value) * sizeof(wchar_t));
    ASSERT(position() < size());
    *position_ = '\0';
    return *this;
}



void MemoryWriter::setPosition(std::size_t pos)
{
    ASSERT(pos < size_);
    ASSERT(memory_ + pos <= position_);
    position_ = memory_ + pos;
}

void MemoryWriter::write(const char* value)
{
    write((void*)value, strlen(value));
}

void MemoryWriter::write(const void* data, std::size_t size)
{
    ASSERT(memory_ <= position_);
    ASSERT(position() < this->size());
    if(size_ - position() > size){
        memcpy(position_, data, size);
        position_ += size;
    }
    else{
        if(reallocate_){
            realloc(size_ * 2);
            write(data, size);
        }
        else
            RAISE(ErrorRuntime("MemoryWriter overflow!"));
    }
    ASSERT(position() < this->size());
}

void MemoryWriter::write(char c)
{
    if(size_ - position() > 1){
        *(char*)(position_) = c;
        ++position_;
    }
    else{
        if(reallocate_){
            realloc(size_ * 2);
            write(c);
        }
        else
            RAISE(ErrorRuntime("MemoryWriter overflow!"));
    }
    ASSERT(position() < this->size());
}
